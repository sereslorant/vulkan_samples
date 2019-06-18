
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
    
    lrMeshGroupBindings    MeshNode;
    lrUniformBindings      ImageNode;
    lvkProfOptMeshDrawNode OptMeshDrawNode;
    
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
                
                
                OptMeshDrawNode = {};
                OptMeshDrawNode.DrawCount = LKV_PROF_SCENE_DIM * LKV_PROF_SCENE_DIM * LKV_PROF_SCENE_DIM;
                OptMeshDrawNode.Draws.resize(LKV_PROF_SCENE_DIM * LKV_PROF_SCENE_DIM * LKV_PROF_SCENE_DIM);
                
                ImageNode= {};
                ImageNode.UniformBindings = &ImageResource;
                
                ImageNode.Children.resize(1);
                ImageNode.Children[0] = &OptMeshDrawNode;
                
                MeshNode.VertexData = &MeshVertexBuffers;
                MeshNode.IndexData  = &MeshIndexBuffers;
                MeshNode.MeshGroup  = &MeshGroup2Data.MeshGroup;
                
                MeshNode.Children.resize(1);
                MeshNode.Children[0] = &ImageNode;
                /*
                MeshDescriptors.resize(SceneDim * SceneDim * SceneDim);
                MeshUbos.resize(SceneDim * SceneDim * SceneDim);
                MeshDrawCalls.resize(SceneDim * SceneDim * SceneDim);
                */
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
                                OptMeshDrawNode.Draws[BlockId].MeshDescriptors = {
                                    .PipelineLayout = TestRenderer.TestPipelineLayout.PipelineLayout.get(),
                                    .DescriptorSet  = TestRenderer.Descriptors.UboDescriptorSet,
                                    .Offset         = BlockId*TestUniformBuffer.StructSize,
                                    .StructSize     = TestUniformBuffer.StructSize
                                };
                                
                                OptMeshDrawNode.Draws[BlockId].MeshUbos = {};
                                OptMeshDrawNode.Draws[BlockId].MeshUbos.DynamicUniformBindings = &OptMeshDrawNode.Draws[BlockId].MeshDescriptors;
                                //MeshUbos[BlockId].Children.resize(1);
                                //MeshUbos[BlockId].Children[0] = &MeshDrawCalls[BlockId];
                                
                                OptMeshDrawNode.Draws[BlockId].MeshDrawCalls = {};
                                OptMeshDrawNode.Draws[BlockId].MeshDrawCalls.Indexed = true;
                                OptMeshDrawNode.Draws[BlockId].MeshDrawCalls.MeshId  = BlockId % NUM_MODELS;
                                //OptMeshDrawNode.Draws[BlockId].MeshDrawCalls.IndexedSubMesh = MeshGroup2Data.MeshGroup.IndexedSubMeshes[BlockId % NUM_MODELS];
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
                MeshNode.Accept(CommandRecorder);
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
        MeshGroup2Data = {};
        ImageGroup2Data = {};
        
        ResourceManager = nullptr;
        
        vkDeviceWaitIdle(DemoFramework.Device.get());
    }
};

const bool IsResizable = true;

std::unique_ptr<IVKSampleApp> CreateSampleApp(IVKWindow &window)
{
    return std::unique_ptr<IVKSampleApp>(new lvkResourcePoolApp(window));
}
