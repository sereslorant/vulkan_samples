
#include <lvkSampleApps/lvkDemoUtils.h>

#include <lvkUtils/lvkShaderModuleLibrary.h>

#include <lvkUtils/lvkSmartPtrs/lvkSampler.h>

#include <lvkUtils/lvkSmartPtrs/lvkPipelineLayout.h>
#include <lvkUtils/lvkSmartPtrs/lvkDescriptorSetLayout.h>

#include <lvkUtils/lvkDescriptorPoolUtils.h>

#include <lvkResourceManager/lrmIstreamMeshGroupLoader.h>
#include <lvkResourceManager/lrmIstreamImageGroupLoader.h>

#include "../ResourceManager/lvkResourceTestRenderer.h"

#include <lvkSampleApps/glmath.hpp>

#include <condition_variable>

#include "lvkProfileUtils.h"

struct lvkTestCommandBufferTask
{
    VkRenderPass          RenderPass;
    VkFramebuffer         Framebuffer;
    VkCommandBuffer       CommandBuffer;
    lvkSwapchain         *Swapchain;
    VkPipeline            Pipeline;
    const lrResourceNode *ResourceNode;
    
    bool Finished = false;
    
    void Record()
    {
        VkCommandBufferInheritanceInfo InheritanceInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
            .pNext = nullptr,
            .renderPass = RenderPass,
            .subpass = 0,
            .framebuffer = Framebuffer,
            .occlusionQueryEnable = VK_FALSE,
            .queryFlags = 0x0,
            .pipelineStatistics = 0x0
        };
        
        lvkRecordCmdBufferGuard RecordCmdBufferGuard(
            CommandBuffer,
            LVK_PROF_SECONDARY_COMMAND_BUFFER_USAGE_FLAGS,
            &InheritanceInfo
        );
        
        VkViewport Viewport = {};
        Viewport.x = 0.0;
        Viewport.y = 0.0;
        Viewport.width  = Swapchain->GetExtent().width;
        Viewport.height = Swapchain->GetExtent().height;
        Viewport.minDepth = 0.0f;
        Viewport.maxDepth = 1.0f;
        
        VkRect2D Scissor = {};
        Scissor.offset = {0, 0};
        Scissor.extent.width  = Swapchain->GetExtent().width;
        Scissor.extent.height = Swapchain->GetExtent().height;
        
        vkCmdSetViewport(CommandBuffer,0,1,&Viewport);
        vkCmdSetScissor(CommandBuffer,0,1,&Scissor);
        
        vkCmdBindPipeline(CommandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,Pipeline);
        
        lvkHierarchyRecorder CommandRecorder(CommandBuffer);
        ResourceNode->Accept(CommandRecorder);
        
        Finished = true;
    }
};

class lvkRenderingThread
{
private:
    
    bool BgThreadRunning = true;
    std::thread BgThread;
    
    std::mutex              BgThreadMutex;
    std::condition_variable BgThreadVariable;
    
    std::condition_variable &MainThreadVariable;
    
    VkDevice Device;
    
    lvkCommandPool  CommandPool;
    VkCommandBuffer CommandBuffer;
    
    lvkTestCommandBufferTask CommandBufferTask;
    
public:
    
    bool IsFinished()
    {
        return CommandBufferTask.Finished;
    }
    
    VkCommandBuffer GetCommandBuffer()
    {
        return CommandBuffer;
    }
    
    void AddTask(VkRenderPass render_pass,VkFramebuffer framebuffer,lvkSwapchain *swapchain,const lrResourceNode *root_node,VkPipeline pipeline)
    {
        CommandBufferTask.RenderPass   = render_pass;
        CommandBufferTask.Framebuffer  = framebuffer;
        CommandBufferTask.Swapchain    = swapchain;
        CommandBufferTask.Pipeline     = pipeline;
        CommandBufferTask.ResourceNode = root_node;
        
        CommandBufferTask.Finished     = false;
        
        BgThreadVariable.notify_one();
    }
    
    void ResetCommandPool()
    {
        lvkCommandPoolUtil CommandPoolUtil(Device,CommandPool.get());
        CommandPoolUtil.Reset();
    }
    
    lvkRenderingThread(VkDevice device,std::uint32_t graphics_queue_family_index,std::condition_variable &main_thread_variable)
        :MainThreadVariable(main_thread_variable),Device(device)
    {
        CommandPool = lvkCommandPoolUtil::CreateCommandPool(
            device,
            graphics_queue_family_index,
            LVK_PROF_COMMAND_POOL_CREATE_FLAGS
        );
        
        lvkCommandPoolUtil CommandPoolUtil(Device,CommandPool.get());
        CommandBuffer = CommandPoolUtil.Allocate(VK_COMMAND_BUFFER_LEVEL_SECONDARY);
        CommandBufferTask = {
            .RenderPass    = VK_NULL_HANDLE,
            .Framebuffer   = VK_NULL_HANDLE,
            .CommandBuffer = CommandBuffer,
            .Swapchain     = nullptr,
            .Pipeline      = VK_NULL_HANDLE,
            .ResourceNode  = nullptr,
            .Finished      = true
        };
        
        BgThread = std::thread(
            [this]()
            {
                while(BgThreadRunning)
                {
                    {
                        std::unique_lock<std::mutex> ThreadGuard(BgThreadMutex);
                        BgThreadVariable.wait_for(ThreadGuard,std::chrono::milliseconds(2));
                    }
                    
                    if(!CommandBufferTask.Finished)
                    {
                        CommandBufferTask.Record();
                    }
                    
                    MainThreadVariable.notify_one();
                }
            }
        );
    }
    
    ~lvkRenderingThread()
    {
        BgThreadRunning = false;
        BgThread.join();
    }
};

class lvkResourcePoolApp : public IVKSampleApp
{
private:
    
    lvkDemoFramework DemoFramework;
    
    lvkShaderModuleLibrary ShaderModuleLibrary;
    
    std::unique_ptr<lvkSwapchain> Swapchain;
    lvkRenderPass RenderPass;
    std::unique_ptr<lvkDemoSwapchainFramebuffers> DemoSwapchainFramebuffers;
    
    lvkPipeline Pipeline;
    
    lvkTestUniformBuffer TestUniformBuffer;
    
    
    struct lTransform
    {
        mat4 MvpMatrix;
        vec4 MatData;
    };
    
    float RotX = 0.0;
    float RotY = 0.0;
    
    /*
     * Pools
     */
    
    lvkTestPools TestPools;
    
    // Loaded data
    
    lvkFileMeshGroupLoader DeinterleavedMeshGroupLoader = {MeshNames.data(),MeshNames.size()};
    
    lvkFileImageGroupLoader ImageGroupLoader = {ImageFileNames.data(),ImageFileNames.size()};
    
    
    lvkMeshGroupData  MeshGroup2Data;
    lvkImageGroupData ImageGroup2Data;
    
    
    constexpr static float Step = 4.0;
    constexpr static float Edge = Step * (LKV_PROF_SCENE_DIM/2);
    
    bool MeshInstancesCreated = false;
    
    lvkMeshGroupVertexData MeshVertexBuffers;
    lvkMeshGroupIndexData  MeshIndexBuffers;
    
    lvkDescriptorSet  ImageResource;
    
    std::vector<lrMeshGroupBindings>    MeshNodes;
    std::vector<lrUniformBindings>      ImageNodes;
    std::vector<lvkProfOptMeshDrawNode> OptMeshDrawNode;
    
    bool ResourcesLoaded()
    {
        return MeshGroup2Data.Loaded  && ImageGroup2Data.Loaded;
    }
    
    
    // Rendering
    
    bool RendererInitialized = false;
    lvkTestRenderer TestRenderer;
    
    /*
     * Test resource definitions
     */
    
    std::size_t ThreadCount = std::thread::hardware_concurrency();
    
    lvkCommandPool  CommandPool;
    VkCommandBuffer CommandBuffer;
    
    std::mutex MainThreadMutex;
    std::condition_variable MainThreadVariable;
    
    std::vector<std::unique_ptr<lvkRenderingThread> > RenderingThreads;
    std::vector<VkCommandBuffer> BgCommandBuffers;
    
    bool EveryBufferReady()
    {
        bool Finished = true;
        
        for(auto &RenderingThread : RenderingThreads)
        {
            if(!RenderingThread->IsFinished())
                {Finished = false;}
        }
        
        return Finished;
    }
    
    enum lvkDemoSemaphoreId
    {
        LVK_DEMO_IMG_AVAILABLE      = 0,
        LVK_DEMO_RENDERING_FINISHED = 1,
    };
    lvkSemaphore Semaphores[2];
    lvkFence FrameFence;
    
    std::unique_ptr<lvkGpuCommandSubmitTask> GraphicsSubmitTask;
    std::unique_ptr<lvkGpuPresentSubmitTask> PresentSubmitTask;
    
    std::unique_ptr<lvkResourceManager> ResourceManager;
    
    lvkProfRenderCounter ProfRenderCounter;
    lvkProfRenderCounter ProfRecordCounter;
    
public:
    
    virtual void Loop() override
    {
        if(ResourceManager != nullptr)
            {ResourceManager->Run();}
        
        /*
         * Uniform upload
         */
        
        if(ResourcesLoaded())
        {
            if(!RendererInitialized)
            {
                TestRenderer.Descriptors.Initialize(
                    DemoFramework.Device.get(),
                    TestRenderer.TestPipelineLayout.UniformBufferLayout.get(),
                    TestRenderer.TestPipelineLayout.TextureArrayLayout.get(),
                    TestPools.TextureCategory->Instances.size()
                );
                
                TestRenderer.WritePoolDescriptorSets(DemoFramework.Device.get(),*TestPools.TextureCategory.get());
                
                TestUniformBuffer.WriteDescriptorSet(DemoFramework.Device.get(),TestRenderer.Descriptors.UboDescriptorSet);
                
                std::size_t ThreadBlockSize = 1 + (LKV_PROF_SCENE_DIM * LKV_PROF_SCENE_DIM * LKV_PROF_SCENE_DIM) / ThreadCount;
                
                ImageResource = {
                    .PipelineLayout = TestRenderer.TestPipelineLayout.PipelineLayout.get(),
                    .DescriptorSet  = TestRenderer.GetImageGroupDescriptor(ImageGroup2Data)
                };
                
                std::array<VkBuffer,lvkMeshGroupVertexData::MAX_BUFFERS>     VertexBuffers;
                std::array<VkDeviceSize,lvkMeshGroupVertexData::MAX_BUFFERS> BufferOffsets;
                for(std::size_t j=0;j < MeshGroup2Data.BindingCount;j++)
                {
                    VertexBuffers[j] = MeshGroup2Data.VertexBuffer;
                    BufferOffsets[j] = MeshGroup2Data.BindingsOffsets[j];
                }
                
                MeshVertexBuffers = lvkMeshGroupVertexData{MeshGroup2Data.BindingCount,VertexBuffers,BufferOffsets};
                MeshIndexBuffers  = {MeshGroup2Data.IndexBuffer,MeshGroup2Data.IndexDataRegion.GetOffset()};
                
                OptMeshDrawNode.resize(ThreadCount);
                for(std::size_t i=0;i < OptMeshDrawNode.size();i++)
                {
                    OptMeshDrawNode[i] = {};
                    OptMeshDrawNode[i].Draws.resize(ThreadBlockSize);
                }
                
                ImageNodes.resize(ThreadCount);
                for(std::size_t i=0;i < ImageNodes.size();i++)
                {
                    ImageNodes[i] = {};
                    ImageNodes[i].UniformBindings = &ImageResource;
                    
                    ImageNodes[i].Children.resize(1);
                    ImageNodes[i].Children[0] = &OptMeshDrawNode[i];
                }
                
                MeshNodes.resize(ThreadCount);
                for(std::size_t i=0;i < MeshNodes.size();i++)
                {
                    MeshNodes[i].VertexData = &MeshVertexBuffers;
                    MeshNodes[i].IndexData  = &MeshIndexBuffers;
                    MeshNodes[i].MeshGroup  = &MeshGroup2Data.MeshGroup;
                    
                    MeshNodes[i].Children.resize(1);
                    MeshNodes[i].Children[0] = &ImageNodes[i];
                }
                
                RendererInitialized = true;
            }
            
            {
                lMemoryView FullMemoryView = TestUniformBuffer.MappedMemory.GetView();
                
                lBlockMemoryView TriDataBufferView((char*)FullMemoryView.GetPtr(),LKV_PROF_SCENE_DIM * LKV_PROF_SCENE_DIM * LKV_PROF_SCENE_DIM,TestUniformBuffer.StructSize);
                
                
                for(std::size_t i = 0;i < LKV_PROF_SCENE_DIM;i++)
                {
                    for(std::size_t j=0;j < LKV_PROF_SCENE_DIM;j++)
                    {
                        for(std::size_t k=0;k < LKV_PROF_SCENE_DIM;k++)
                        {
                            std::uint32_t BlockId = j*LKV_PROF_SCENE_DIM*LKV_PROF_SCENE_DIM + i*LKV_PROF_SCENE_DIM + k;
                            
                            lTransform *TriDataElement = (lTransform *)TriDataBufferView.GetBlock(BlockId);
                            TriDataElement->MvpMatrix = rotate_x(RotX) *
                                                        rotate_y(RotY) *
                                                        translate(vec4{i*Step - Edge,j*Step - Edge,-10.0f - k*Step}) *
                                                        perspective<float>(3.14/2.0,1.0,1.0,-200.0);
                            
                            TriDataElement->MatData   = vec4{0,0,0,(float)(BlockId%NUM_IMAGES)};
                            
                            if(!MeshInstancesCreated)
                            {
                                std::size_t ThreadId = BlockId % ThreadCount;
                                
                                std::size_t CurrentId = OptMeshDrawNode[ThreadId].DrawCount;
                                OptMeshDrawNode[ThreadId].DrawCount++;
                                
                                OptMeshDrawNode[ThreadId].Draws[CurrentId].MeshDescriptors = {
                                    .PipelineLayout = TestRenderer.TestPipelineLayout.PipelineLayout.get(),
                                    .DescriptorSet  = TestRenderer.Descriptors.UboDescriptorSet,
                                    .Offset         = BlockId*TestUniformBuffer.StructSize,
                                    .StructSize     = TestUniformBuffer.StructSize
                                };
                                
                                OptMeshDrawNode[ThreadId].Draws[CurrentId].MeshUbos = {};
                                OptMeshDrawNode[ThreadId].Draws[CurrentId].MeshUbos.DynamicUniformBindings = &OptMeshDrawNode[ThreadId].Draws[CurrentId].MeshDescriptors;
                                //MeshUbos[CurrentId].Children.resize(1);
                                //MeshUbos[CurrentId].Children[0] = &OptMeshDrawNode[ThreadId].MeshDrawCalls[CurrentId];
                                
                                OptMeshDrawNode[ThreadId].Draws[CurrentId].MeshDrawCalls = {};
                                OptMeshDrawNode[ThreadId].Draws[CurrentId].MeshDrawCalls.Indexed = true;
                                OptMeshDrawNode[ThreadId].Draws[CurrentId].MeshDrawCalls.MeshId  = BlockId % NUM_MODELS;
                                //MeshDrawCalls[CurrentId].IndexedSubMesh = MeshGroup2Data.MeshGroup.IndexedSubMeshes[BlockId % NUM_MODELS];
                            }
                        }
                    }
                }
            }
            
            MeshInstancesCreated = true;
            
            RotX += 0.01;
            
            if(RotX > 6.28)
                {RotX = 0.0;}
            
            RotY += 0.01;
            
            if(RotY > 6.28)
                {RotY = 0.0;}
        }
        
        if(ResourcesLoaded())
        {
            ResourceManager = nullptr;
            
            if(RenderingThreads.size() == 0)
            {
                CommandPool = lvkCommandPoolUtil::CreateCommandPool(
                    DemoFramework.Device.get(),
                    DemoFramework.GraphicsQueueFamilyIndex,
                    LVK_PROF_COMMAND_POOL_CREATE_FLAGS
                );
                
                lvkCommandPoolUtil CommandPoolUtil(DemoFramework.Device.get(),CommandPool.get());
                CommandBuffer = CommandPoolUtil.Allocate();
                
                RenderingThreads.resize(ThreadCount);
                BgCommandBuffers.resize(ThreadCount);
                
                for(std::size_t i=0;i < RenderingThreads.size();i++)
                {
                    RenderingThreads[i] = std::unique_ptr<lvkRenderingThread>(
                        new lvkRenderingThread(
                            DemoFramework.Device.get(),
                            DemoFramework.GraphicsQueueFamilyIndex,
                            MainThreadVariable
                        )
                    );
                    
                    BgCommandBuffers[i] = RenderingThreads[i]->GetCommandBuffer();
                }
            }
            
            std::uint32_t NextImageIndex;
            
            VkResult Result = vkAcquireNextImageKHR(
                DemoFramework.Device.get(),
                Swapchain->GetSwapchain(),
                std::numeric_limits<uint64_t>::max(),
                Semaphores[LVK_DEMO_IMG_AVAILABLE].get(),
                VK_NULL_HANDLE,
                &NextImageIndex
            );
            
            if(Result == VK_SUBOPTIMAL_KHR || Result == VK_ERROR_OUT_OF_DATE_KHR)
            {
                Swapchain->Recreate();
                
                vkAcquireNextImageKHR(
                    DemoFramework.Device.get(),
                    Swapchain->GetSwapchain(),
                    std::numeric_limits<uint64_t>::max(),
                    Semaphores[LVK_DEMO_IMG_AVAILABLE].get(),
                    VK_NULL_HANDLE,
                    &NextImageIndex
                );
            }
            
            lvkCommandPoolUtil CommandPoolUtil(DemoFramework.Device.get(),CommandPool.get());
            CommandPoolUtil.Reset();
            
            for(std::size_t i=0;i < RenderingThreads.size();i++)
            {
                RenderingThreads[i]->ResetCommandPool();
            }
            
            ProfRenderCounter.BeginRecording();
            
            //Begin recording secondary cmd buffers
            ProfRecordCounter.BeginRecording();
            
            
            for(std::size_t i=0;i < RenderingThreads.size();i++)
            {
                RenderingThreads[i]->AddTask(
                    RenderPass.get(),
                    DemoSwapchainFramebuffers->GetFramebuffer(NextImageIndex),
                    Swapchain.get(),
                    &MeshNodes[i],
                    Pipeline.get()
                );
            }
            
            while(!EveryBufferReady())
            {
                // Wait
                std::unique_lock<std::mutex> MainThreadGuard(MainThreadMutex);
                MainThreadVariable.wait(MainThreadGuard);
            }
            
            //Finish recording secondary cmd buffers, begin primary
            ProfRecordCounter.FinishRecording();
            
            {
                lvkRecordCmdBufferGuard RecordCmdBufferGuard(
                    CommandBuffer,
                    LVK_PROF_PRIMARY_COMMAND_BUFFER_USAGE_FLAGS
                );
                
                lvkDemoRecordSimpleRenderPassGuard RecordSimpleRenderPassGuard(
                    CommandBuffer,
                    RenderPass.get(),
                    DemoSwapchainFramebuffers->GetFramebuffer(NextImageIndex),
                    Swapchain->GetExtent(),
                    true,
                    VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS
                );
                
                vkCmdExecuteCommands(CommandBuffer,BgCommandBuffers.size(),BgCommandBuffers.data());
            }
            //Finish recording primary cmd buffers
            ProfRecordCounter.FinishSubmission();
            
            ProfRenderCounter.FinishRecording();
            
            GraphicsSubmitTask->Submit(1,&CommandBuffer,FrameFence.get());
            
            VkSwapchainKHR Swapchains[] = {Swapchain->GetSwapchain()};
            std::uint32_t  ImgIndices[] = {NextImageIndex};
            PresentSubmitTask->Submit(1,Swapchains,ImgIndices);
            
            std::array<VkFence,1> Fences = {FrameFence.get()};
            vkWaitForFences(
                DemoFramework.Device.get(),
                1,
                Fences.data(),
                VK_TRUE,
                std::numeric_limits<uint64_t>::max()
            );
            
            ProfRenderCounter.FinishSubmission();
            ProfRenderCounter.Print();
            
            std::cout << "Recording secondary cmd buffers: " << ProfRecordCounter.GetCmdRecord() << std::endl;
            std::cout << "Recording primary cmd buffers: "   << ProfRecordCounter.GetExec() << std::endl;
            
            std::cout << "Last average recording secondary cmd buffers: " << ProfRecordCounter.GetCmdAverage() << std::endl;
            std::cout << "Last average recording primary cmd buffers: "   << ProfRecordCounter.GetExecAverage() << std::endl;
            
            vkResetFences(
                DemoFramework.Device.get(),
                1,
                Fences.data()
            );
        }
    }
    
    lvkResourcePoolApp(IVKWindow &window)
        :DemoFramework(window,LVK_PROF_TURN_ON_VALIDATION)
    {
        ShaderModuleLibrary = {DemoFramework.Device.get()};
        
        Swapchain = std::unique_ptr<lvkSwapchain>(new lvkSwapchain(DemoFramework.Device.get(),DemoFramework.PhysicalDevice,DemoFramework.Surface.get()));
        bool HasDepthBuffer = true;
        
        lvkMemoryUtility MemoryUtility(DemoFramework.Device.get(),DemoFramework.PhysicalDevice);
        
        RenderPass = DemoFramework.CreateSimpleRenderPass(DemoFramework.Device.get(),Swapchain->GetFormat(),HasDepthBuffer);
        DemoSwapchainFramebuffers = std::unique_ptr<lvkDemoSwapchainFramebuffers>(
            new lvkDemoSwapchainFramebuffers(
                DemoFramework.Device.get(),
                RenderPass.get(),
                *Swapchain.get(),
                 MemoryUtility.CreateAllocator(LVK_STATIC_MEMORY_PROPERTIES),
                 HasDepthBuffer
            )
        );
        
        TestRenderer.TestPipelineLayout.Initialize(DemoFramework.Device.get());
        
        Pipeline = TestRenderer.CreateInterleavedPipeline(
            DemoFramework.Device.get(),
            RenderPass.get(),
            0,
            ShaderModuleLibrary.GetShaderModule("../Shaders/MeshVertexShader.spv"),
            ShaderModuleLibrary.GetShaderModule("../Shaders/MeshTexCoordFragmentShader.spv")
        );
        
        TestPools.Initialize(
            DemoFramework.Device.get(),
            DemoFramework.PhysicalDevice,
            {
                .MeshBufferPoolSize    = 16777216,
                .StagingBufferPoolSize = 33554432,
                .TextureWidth  = 128,
                .TextureHeight = 128,
                .TextureCount  = NUM_IMAGES,
                .TextureFormat = VK_FORMAT_R32G32B32A32_SFLOAT
            }
        );
        
        TestUniformBuffer.Initialize(
            DemoFramework.Device.get(),
            DemoFramework.PhysicalDevice,
            sizeof(lTransform),
            LKV_PROF_SCENE_DIM * LKV_PROF_SCENE_DIM * LKV_PROF_SCENE_DIM
        );
        
        ResourceManager = std::unique_ptr<lvkResourceManager>(
            new  lvkResourceManager(
                DemoFramework.Device.get(),
                DemoFramework.GraphicsQueue,
                DemoFramework.GraphicsQueueFamilyIndex,
                *TestPools.VertexBufferCategory,
                *TestPools.IndexBufferCategory,
                *TestPools.TextureCategory,
                *TestPools.StagingBufferCategory
            )
        );
        /*
        ResourceManager->AddTask(
            {{&InterleavedMeshGroupLoader,&MeshGroup1Data},{&DeinterleavedMeshGroupLoader,&MeshGroup2Data}},
            {{&ImageGroupLoader1,&ImageGroup1Data},{&ImageGroupLoader2,&ImageGroup2Data}}
        );
        
        ResourceManager->AddTask(
            {{&InterleavedMeshGroupLoader,&MeshGroup1Data}},
            {{&ImageGroupLoader1,&ImageGroup1Data}}
        );
        */
        
        ResourceManager->AddTask(
            {{&DeinterleavedMeshGroupLoader,&MeshGroup2Data}},
            {{&ImageGroupLoader,&ImageGroup2Data}}
        );
        
        {
            Semaphores[LVK_DEMO_IMG_AVAILABLE] = lvkSemaphore{lvkGpuSubmission::CreateSemaphore(DemoFramework.Device.get()),{DemoFramework.Device.get()}};
            Semaphores[LVK_DEMO_RENDERING_FINISHED] = lvkSemaphore{lvkGpuSubmission::CreateSemaphore(DemoFramework.Device.get()),{DemoFramework.Device.get()}};
            
            GraphicsSubmitTask = std::unique_ptr<lvkGpuCommandSubmitTask>(new lvkGpuCommandSubmitTask(DemoFramework.GraphicsQueue,1,1));
            PresentSubmitTask  = std::unique_ptr<lvkGpuPresentSubmitTask>(new lvkGpuPresentSubmitTask(DemoFramework.PresentQueue,1));
            
            GraphicsSubmitTask->AddDependency(Semaphores[LVK_DEMO_IMG_AVAILABLE].get(),VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
            GraphicsSubmitTask->AddSignalSemaphore(Semaphores[LVK_DEMO_RENDERING_FINISHED].get());
            
            PresentSubmitTask->AddDependency(Semaphores[LVK_DEMO_RENDERING_FINISHED].get());
            
            FrameFence = lvkGpuSubmission::CreateFence(DemoFramework.Device.get(),0x0);
        }
    }
    
    ~lvkResourcePoolApp()
    {
        MeshGroup2Data = {};
        ImageGroup2Data = {};
        
        
        for(std::size_t i = 0;i < RenderingThreads.size();i++)
        {
            RenderingThreads[i] = nullptr;
        }
        
        vkDeviceWaitIdle(DemoFramework.Device.get());
    }
};

const bool IsResizable = true;

std::unique_ptr<IVKSampleApp> CreateSampleApp(IVKWindow &window)
{
    return std::unique_ptr<IVKSampleApp>(new lvkResourcePoolApp(window));
}
