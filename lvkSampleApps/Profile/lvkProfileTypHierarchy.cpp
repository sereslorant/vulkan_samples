
#include <lvkSampleApps/lvkDemoUtils.h>

#include <lvkUtils/lvkShaderModuleLibrary.h>

#include <lvkUtils/lvkSmartPtrs/lvkSampler.h>

#include <lvkUtils/lvkSmartPtrs/lvkPipelineLayout.h>
#include <lvkUtils/lvkSmartPtrs/lvkDescriptorSetLayout.h>

#include <lvkUtils/lvkDescriptorPoolUtils.h>

#include <lvkResourceManager/lrmIstreamMeshGroupLoader.h>
#include <lvkResourceManager/lrmIstreamImageGroupLoader.h>

#include "../ResourceManager/lvkResourceTestRenderer.h"

#include <lvkRenderer/lvkResourceHierarchy.h>

#include <lvkSampleApps/glmath.hpp>

#include "lvkProfileUtils.h"

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
    
    lvkProfFileLoaderArray FileLoaderArray;
    
    lvkMeshGroupData  MeshGroupData[NUM_MODELS];
    lvkImageGroupData ImageGroupData[NUM_IMAGES];
    
    VkDescriptorSet   ImageDescriptorSets[NUM_IMAGES];
    
    constexpr static float Step = 4.0;
    constexpr static float Edge = Step * (LKV_PROF_SCENE_DIM/2);
    
    bool MeshInstancesCreated = false;
    
    lvkMeshGroupVertexData MeshVertexBuffers[NUM_MODELS];
    lvkMeshGroupIndexData  MeshIndexBuffers[NUM_MODELS];
    
    lvkDescriptorSet  ImageResource[NUM_IMAGES];
    
    lrResourceNode RootNode = {};
    
    lrMeshGroupBindings MeshNodes[NUM_MODELS] = {};
    lrUniformBindings   ImageNodes[NUM_MODELS][NUM_IMAGES] = {};
    
    lvkProfOptMeshDrawNode OptMeshDrawNode[NUM_MODELS][NUM_IMAGES] = {};
    
    bool ResourcesLoaded()
    {
        bool Loaded = true;
        
        for(auto &ImgGroup : ImageGroupData)
        {
            if(!ImgGroup.Loaded)
                {Loaded = false;}
        }
        
        for(auto &MeshGroup : MeshGroupData)
        {
            if(!MeshGroup.Loaded)
                {Loaded = false;}
        }
        
        return Loaded;
    }
    
    
    // Rendering
    
    bool RendererInitialized = false;
    lvkTestRenderer TestRenderer;
    
    /*
     * Test resource definitions
     */
    
    lvkCommandPool  CommandPool;
    VkCommandBuffer CommandBuffer;
    
    //lvkDemoTaskGraph TaskGraph;
    
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
            ResourceManager = nullptr;
            
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
                
                for(std::size_t i=0;i < NUM_IMAGES;i++)
                {
                    ImageDescriptorSets[i] = TestRenderer.GetImageGroupDescriptor(ImageGroupData[i]);
                    ImageResource[i] = {
                        .PipelineLayout = TestRenderer.TestPipelineLayout.PipelineLayout.get(),
                        .DescriptorSet  = ImageDescriptorSets[i]
                    };
                    
                    for(std::size_t j=0;j < NUM_MODELS;j++)
                    {
                        //std::size_t DrawCallSegmentSize = (LKV_PROF_SCENE_DIM*LKV_PROF_SCENE_DIM*LKV_PROF_SCENE_DIM) / (NUM_MODELS*NUM_IMAGES) + 1;
                        OptMeshDrawNode[j][i] = {};
                        //OptMeshDrawNode[j][i].Draws.resize(DrawCallSegmentSize);
                        
                        ImageNodes[j][i] = {};
                        ImageNodes[j][i].UniformBindings = &ImageResource[i];
                        
                        ImageNodes[j][i].Children.resize(1);
                        ImageNodes[j][i].Children[0] = &OptMeshDrawNode[j][i];
                    }
                }
                
                RootNode.Children.resize(NUM_MODELS);
                for(std::size_t i=0;i < NUM_MODELS;i++)
                {
                    RootNode.Children[i] = &MeshNodes[i];
                    
                    std::array<VkBuffer,lvkMeshGroupVertexData::MAX_BUFFERS>     VertexBuffers;
                    std::array<VkDeviceSize,lvkMeshGroupVertexData::MAX_BUFFERS> BufferOffsets;
                    for(std::size_t j=0;j < MeshGroupData[i].BindingCount;j++)
                    {
                        VertexBuffers[j] = MeshGroupData[i].VertexBuffer;
                        BufferOffsets[j] = MeshGroupData[i].BindingsOffsets[j];
                    }
                    
                    MeshVertexBuffers[i] = lvkMeshGroupVertexData{MeshGroupData[i].BindingCount,VertexBuffers,BufferOffsets};
                    MeshIndexBuffers[i]  = {MeshGroupData[i].IndexBuffer,MeshGroupData[i].IndexDataRegion.GetOffset()};
                    
                    MeshNodes[i].VertexData = &MeshVertexBuffers[i];
                    MeshNodes[i].IndexData  = &MeshIndexBuffers[i];
                    MeshNodes[i].MeshGroup  = &MeshGroupData[i].MeshGroup;
                    
                    MeshNodes[i].Children.resize(NUM_IMAGES);
                    for(std::size_t j=0;j < NUM_IMAGES;j++)
                    {
                        MeshNodes[i].Children[j] = &ImageNodes[i][j];
                    }
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
                            
                            TriDataElement->MatData   = vec4{0,0,0,0};
                            
                            if(!MeshInstancesCreated)
                            {
                                std::size_t MeshId    = BlockId % NUM_MODELS;
                                std::size_t TextureId = BlockId % NUM_IMAGES;
                                
                                std::size_t CurrentId = OptMeshDrawNode[MeshId][TextureId].DrawCount;
                                OptMeshDrawNode[MeshId][TextureId].DrawCount++;
                                
                                OptMeshDrawNode[MeshId][TextureId].Draws.push_back({});
                                
                                OptMeshDrawNode[MeshId][TextureId].Draws[CurrentId].MeshDescriptors = {
                                    .PipelineLayout = TestRenderer.TestPipelineLayout.PipelineLayout.get(),
                                    .DescriptorSet  = TestRenderer.Descriptors.UboDescriptorSet,
                                    .Offset         = BlockId*TestUniformBuffer.StructSize,
                                    .StructSize     = TestUniformBuffer.StructSize
                                };
                                
                                OptMeshDrawNode[MeshId][TextureId].Draws[CurrentId].MeshUbos = {};
                                //OptMeshDrawNode[MeshId][TextureId].Draws[CurrentId].MeshUbos.DynamicUniformBindings = &OptMeshDrawNode[MeshId][TextureId].Draws[CurrentId].MeshDescriptors;
                                //OptMeshDrawNode[MeshId][TextureId].Draws[CurrentId].MeshUbos.Children.resize(1);
                                //OptMeshDrawNode[MeshId][TextureId].Draws[CurrentId].MeshUbos.Children[0] = &OptMeshDrawNode[MeshId][TextureId].Draws[CurrentId].MeshDrawCalls;
                                
                                OptMeshDrawNode[MeshId][TextureId].Draws[CurrentId].MeshDrawCalls = {};
                                OptMeshDrawNode[MeshId][TextureId].Draws[CurrentId].MeshDrawCalls.Indexed = true;
                                OptMeshDrawNode[MeshId][TextureId].Draws[CurrentId].MeshDrawCalls.MeshId  = 0;
                                //MeshDrawCalls[CurrentId].IndexedSubMesh = MeshGroupData[MeshId].MeshGroup.IndexedSubMeshes[0];
                                
                                
                                //ImageNodes[MeshId][TextureId].Children.push_back(&MeshUbos[CurrentId]);
                            }
                        }
                    }
                }
            }
            
            if(!MeshInstancesCreated)
            {
                for(std::size_t i=0;i < NUM_MODELS;i++)
                {
                    for(std::size_t j=0;j < NUM_IMAGES;j++)
                    {
                        for(auto &Draw : OptMeshDrawNode[i][j].Draws)
                        {
                            Draw.MeshUbos.DynamicUniformBindings = &Draw.MeshDescriptors;
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
            
            ProfRenderCounter.BeginRecording();
            
            {
                lvkRecordCmdBufferGuard RecordCmdBufferGuard(CommandBuffer,LVK_PROF_PRIMARY_COMMAND_BUFFER_USAGE_FLAGS);
                
                lvkDemoRecordSimpleRenderPassGuard RecordSimpleRenderPassGuard(
                    CommandBuffer,
                    RenderPass.get(),
                    DemoSwapchainFramebuffers->GetFramebuffer(NextImageIndex),
                    Swapchain->GetExtent(),
                    true
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
                
                vkCmdBindPipeline(CommandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,Pipeline.get());
                
                lvkHierarchyRecorder CommandRecorder(CommandBuffer);
                RootNode.Accept(CommandRecorder);
            }
            
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
                .TextureCount  = 1,
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
        
        for(std::size_t i=0;i < NUM_MODELS;i++)
        {
            ResourceManager->AddTask(
                {{&FileLoaderArray.MeshGroupLoader[i],&MeshGroupData[i]}},
                {}
            );
        }
        
        for(std::size_t i=0;i < NUM_IMAGES;i++)
        {
            ResourceManager->AddTask(
                {},
                {{&FileLoaderArray.ImageGroupLoader[i],&ImageGroupData[i]}}
            );
        }
        
        CommandPool = lvkCommandPoolUtil::CreateCommandPool(
            DemoFramework.Device.get(),
            DemoFramework.GraphicsQueueFamilyIndex,
            LVK_PROF_COMMAND_POOL_CREATE_FLAGS
        );
        
        lvkCommandPoolUtil CommandPoolUtil(DemoFramework.Device.get(),CommandPool.get());
        CommandBuffer = CommandPoolUtil.Allocate();
        
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
        for(auto &ImgGroup : ImageGroupData)
        {
            ImgGroup = {};
        }
        
        for(auto &MeshGroup : MeshGroupData)
        {
            MeshGroup = {};
        }
        
        ResourceManager = nullptr;
        
        vkDeviceWaitIdle(DemoFramework.Device.get());
    }
};

const bool IsResizable = true;

std::unique_ptr<IVKSampleApp> CreateSampleApp(IVKWindow &window)
{
    return std::unique_ptr<IVKSampleApp>(new lvkResourcePoolApp(window));
}
