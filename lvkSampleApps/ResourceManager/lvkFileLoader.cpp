
#include <lvkSampleApps/lvkDemoUtils.h>

#include <lvkUtils/lvkShaderModuleLibrary.h>

#include <lvkUtils/lvkSmartPtrs/lvkSampler.h>

#include <lvkUtils/lvkSmartPtrs/lvkPipelineLayout.h>
#include <lvkUtils/lvkSmartPtrs/lvkDescriptorSetLayout.h>

#include <lvkUtils/lvkDescriptorPoolUtils.h>

#include <lvkResourceManager/lrmIstreamMeshGroupLoader.h>
#include <lvkResourceManager/lrmIstreamImageGroupLoader.h>

#include "lvkResourceTestRenderer.h"

#include <lvkSampleApps/glmath.hpp>

class lvkFileLoader : public IVKSampleApp
{
private:
    
    lvkDemoFramework DemoFramework;
    
    lvkShaderModuleLibrary ShaderModuleLibrary;
    
    std::unique_ptr<lvkSwapchain> Swapchain;
    lvkRenderPass RenderPass;
    std::unique_ptr<lvkDemoSwapchainFramebuffers> DemoSwapchainFramebuffers;
    
    lvkPipeline DeinterleavedPipelineNrm;
    lvkPipeline InterleavedPipelineNrm;
    
    lvkPipeline DeinterleavedPipelineTex;
    lvkPipeline InterleavedPipelineTex;
    
    lvkTestUniformBuffer TestUniformBuffer;
    
    
    struct lTransform
    {
        mat4 MvpMatrix;
        vec4 MatData;
    };
    
    constexpr static std::size_t NUM_MODELS = 4;
    
    constexpr static unsigned int UBO_COUNT = 4;
    
    float RotX = 0.0;
    float RotY = 0.0;
    
    /*
     * Pools
     */
    
    lvkTestPools TestPools;
    
    // Loaded data
    
    
    constexpr static std::array<const char *,4> InterleavedFileNames =
    {
        "../Content/Cone_per_face_normals_interleaved",
        "../Content/Cone_smooth_normals_interleaved",
        "../Content/Cube_per_face_normals_interleaved",
        "../Content/Cube_smooth_normals_interleaved"
    };
    
    constexpr static std::array<const char *,4> DeinterleavedFileNames =
    {
        "../Content/Cone_per_face_normals",
        "../Content/Cone_smooth_normals",
        "../Content/Cube_per_face_normals",
        "../Content/Cube_smooth_normals"
    };
    
    lvkFileMeshGroupLoader InterleavedMeshGroupLoader   = {InterleavedFileNames.data(),  InterleavedFileNames.size()};
    lvkFileMeshGroupLoader DeinterleavedMeshGroupLoader = {DeinterleavedFileNames.data(),DeinterleavedFileNames.size()};
    
    constexpr static std::array<const char *,4> ImageFileNames =
    {
        "../Content/TestImage0",
        "../Content/TestImage1",
        "../Content/TestImage2",
        "../Content/TestImage3"
    };
    
    lvkFileImageGroupLoader ImageGroupLoader1 = {ImageFileNames.data(),ImageFileNames.size()};
    lvkFileImageGroupLoader ImageGroupLoader2 = {ImageFileNames.data(),ImageFileNames.size()};
    
    lvkMeshGroupData MeshGroup1Data;
    lvkMeshGroupData MeshGroup2Data;
    
    lvkImageGroupData ImageGroup1Data;
    lvkImageGroupData ImageGroup2Data;
    
    bool ResourcesLoaded()
    {
        return MeshGroup1Data.Loaded  &&
               MeshGroup2Data.Loaded  &&
               ImageGroup1Data.Loaded &&
               ImageGroup2Data.Loaded;
    }
    
    
    // Rendering
    
    lvkTestRenderer TestRenderer;
    
    /*
     * Test resource definitions
     */
    
    bool CmdBuffersAllocated = false;
    
    std::vector<VkCommandBuffer> CommandBuffers;
    std::unique_ptr<lvkDemoCommandPool> CommandPoolResetObserver;
    std::vector<std::unique_ptr<lvkDemoBufferRecorder> > CommandRecorders;
    
    lvkDemoTaskGraph TaskGraph;
    
    std::unique_ptr<lvkResourceManager> ResourceManager;
    
public:
    
    virtual void Loop() override
    {
        ResourceManager->Run();
        
        
        /*
         * Uniform upload
         */
        
        if(CmdBuffersAllocated)
        {
            float Scale = 0.125;
            
            {
                lMemoryView FullMemoryView = TestUniformBuffer.MappedMemory.GetView();
                
                lBlockMemoryView TriDataBufferView((char*)FullMemoryView.GetPtr(),NUM_MODELS * UBO_COUNT,TestUniformBuffer.StructSize);
                
                for(int j = 0;j < UBO_COUNT;j++)
                {
                    for(int i=0;i < NUM_MODELS;i++)
                    {
                        lTransform *TriDataElement = (lTransform *)TriDataBufferView.GetBlock(j*NUM_MODELS+i);
                        TriDataElement->MvpMatrix = scale(vec4{Scale,Scale,Scale}) * rotate_x(RotX) * rotate_y(RotY) * translate(vec4{-0.8f + i*0.4f,0.8f - j*0.4f,0.5});
                        TriDataElement->MatData   = vec4{0,0,0,(float)i};
                    }
                }
            }
            
            RotX += 0.01;
            
            if(RotX > 6.28)
                {RotX = 0.0;}
            
            RotY += 0.01;
            
            if(RotY > 6.28)
                {RotY = 0.0;}
        }
        
        if(!CmdBuffersAllocated && ResourcesLoaded())
        {
            CmdBuffersAllocated = true;
            
            TestUniformBuffer.Initialize(
                DemoFramework.Device.get(),
                DemoFramework.PhysicalDevice,
                sizeof(lTransform),
                NUM_MODELS * UBO_COUNT
            );
            
            {
                TestRenderer.Descriptors.Initialize(
                    DemoFramework.Device.get(),
                    TestRenderer.TestPipelineLayout.UniformBufferLayout.get(),
                    TestRenderer.TestPipelineLayout.TextureArrayLayout.get(),
                    TestPools.TextureCategory->Instances.size()
                );
                
                TestRenderer.WritePoolDescriptorSets(DemoFramework.Device.get(),*TestPools.TextureCategory.get());
                
                TestUniformBuffer.WriteDescriptorSet(DemoFramework.Device.get(),TestRenderer.Descriptors.UboDescriptorSet);
            }
            
            CommandPoolResetObserver = std::unique_ptr<lvkDemoCommandPool>(
                new lvkDemoCommandPool(DemoFramework.Device.get(),DemoFramework.GraphicsQueueFamilyIndex,0x0,Swapchain.get())
                );
            
            CommandBuffers = CommandPoolResetObserver->Allocate(Swapchain->GetImageCount());
            CommandRecorders.resize(CommandBuffers.size());
            for(int i=0;i < CommandRecorders.size();i++)
            {
                CommandRecorders[i] = std::unique_ptr<lvkDemoBufferRecorder>(
                    new lvkDemoBufferRecorder(
                        [this,i]()
                        {
                            lvkRecordCmdBufferGuard RecordCmdBufferGuard(CommandBuffers[i],VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
                            
                            lvkDemoRecordSimpleRenderPassGuard RecordSimpleRenderPassGuard(
                                CommandBuffers[i],
                                RenderPass.get(),
                                DemoSwapchainFramebuffers->GetFramebuffer(i),
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
                            
                            vkCmdSetViewport(CommandBuffers[i],0,1,&Viewport);
                            vkCmdSetScissor(CommandBuffers[i],0,1,&Scissor);
                            
                            std::size_t StructId = 0;
                            
                            {
                                vkCmdBindPipeline(CommandBuffers[i],VK_PIPELINE_BIND_POINT_GRAPHICS,InterleavedPipelineNrm.get());
                                
                                TestRenderer.BindMeshGroup(CommandBuffers[i],MeshGroup1Data);
                                
                                TestRenderer.BindImageGroup(CommandBuffers[i],ImageGroup1Data);
                                
                                for(int j=0;j < NUM_MODELS;j++)
                                {
                                    TestUniformBuffer.BindBlock(
                                        CommandBuffers[i],
                                        TestRenderer.TestPipelineLayout.PipelineLayout.get(),
                                        StructId
                                    );
                                    StructId += 1;
                                    
                                    for(int k = 0;k < MeshGroup1Data.MeshGroup.Meshes[j].IndexedSubMeshCount;k++)
                                        {lvkTestRenderer::DrawIndexedMesh(CommandBuffers[i],MeshGroup1Data,j,k);}
                                }
                                
                                vkCmdBindPipeline(CommandBuffers[i],VK_PIPELINE_BIND_POINT_GRAPHICS,InterleavedPipelineTex.get());
                                
                                for(int j=0;j < NUM_MODELS;j++)
                                {
                                    TestUniformBuffer.BindBlock(
                                        CommandBuffers[i],
                                        TestRenderer.TestPipelineLayout.PipelineLayout.get(),
                                        StructId
                                    );
                                    StructId += 1;
                                    
                                    for(int k = 0;k < MeshGroup1Data.MeshGroup.Meshes[j].IndexedSubMeshCount;k++)
                                        {lvkTestRenderer::DrawIndexedMesh(CommandBuffers[i],MeshGroup1Data,j,k);}
                                }
                            }
                            
                            {
                                vkCmdBindPipeline(CommandBuffers[i],VK_PIPELINE_BIND_POINT_GRAPHICS,DeinterleavedPipelineNrm.get());
                                
                                TestRenderer.BindMeshGroup(CommandBuffers[i],MeshGroup2Data);
                                
                                TestRenderer.BindImageGroup(CommandBuffers[i],ImageGroup2Data);
                                
                                for(int j=0;j < NUM_MODELS;j++)
                                {
                                    TestUniformBuffer.BindBlock(
                                        CommandBuffers[i],
                                        TestRenderer.TestPipelineLayout.PipelineLayout.get(),
                                        StructId
                                    );
                                    StructId += 1;
                                    
                                    for(int k = 0;k < MeshGroup2Data.MeshGroup.Meshes[j].IndexedSubMeshCount;k++)
                                        {lvkTestRenderer::DrawIndexedMesh(CommandBuffers[i],MeshGroup2Data,j,k);}
                                }
                                
                                vkCmdBindPipeline(CommandBuffers[i],VK_PIPELINE_BIND_POINT_GRAPHICS,DeinterleavedPipelineTex.get());
                                
                                for(int j=0;j < NUM_MODELS;j++)
                                {
                                    TestUniformBuffer.BindBlock(
                                        CommandBuffers[i],
                                        TestRenderer.TestPipelineLayout.PipelineLayout.get(),
                                        StructId
                                    );
                                    StructId += 1;
                                    
                                    for(int k = 0;k < MeshGroup2Data.MeshGroup.Meshes[j].IndexedSubMeshCount;k++)
                                        {lvkTestRenderer::DrawIndexedMesh(CommandBuffers[i],MeshGroup2Data,j,k);}
                                }
                            }
                        },
                        *Swapchain.get()
                    )
                );
            }
        }
        
        if(CmdBuffersAllocated)
        {
            TaskGraph.Submit(DemoFramework.Device.get(),*Swapchain.get(),CommandBuffers.data());
        }
    }
    
    lvkFileLoader(IVKWindow &window)
        :DemoFramework(window)
    {
        ShaderModuleLibrary = {DemoFramework.Device.get()};
        
        Swapchain = std::unique_ptr<lvkSwapchain>(new lvkSwapchain(DemoFramework.Device.get(),DemoFramework.PhysicalDevice,DemoFramework.Surface.get()));
        bool HasDepthBuffer = true;
        
        //HasDepthBuffer = false;
        
        
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
        
        DeinterleavedPipelineNrm = TestRenderer.CreateDeinterleavedPipeline(
            DemoFramework.Device.get(),
            RenderPass.get(),
            0,
            ShaderModuleLibrary.GetShaderModule("../Shaders/MeshVertexShader.spv"),
            ShaderModuleLibrary.GetShaderModule("../Shaders/MeshNrmFragmentShader.spv")
        );
        
        DeinterleavedPipelineTex = TestRenderer.CreateDeinterleavedPipeline(
            DemoFramework.Device.get(),
            RenderPass.get(),
            0,
            ShaderModuleLibrary.GetShaderModule("../Shaders/MeshVertexShader.spv"),
            ShaderModuleLibrary.GetShaderModule("../Shaders/MeshTexCoordFragmentShader.spv")
        );
        
        InterleavedPipelineNrm = TestRenderer.CreateInterleavedPipeline(
            DemoFramework.Device.get(),
            RenderPass.get(),
            0,
            ShaderModuleLibrary.GetShaderModule("../Shaders/MeshVertexShader.spv"),
            ShaderModuleLibrary.GetShaderModule("../Shaders/MeshNrmFragmentShader.spv")
        );
        
        InterleavedPipelineTex = TestRenderer.CreateInterleavedPipeline(
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
                .MeshBufferPoolSize    = 65536,
                .StagingBufferPoolSize = 65536,
                .TextureWidth  = 8,
                .TextureHeight = 8,
                .TextureCount  = 4,
                .TextureFormat = VK_FORMAT_R32G32B32A32_SFLOAT
            }
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
        
        ResourceManager->AddTask(
            {{&InterleavedMeshGroupLoader,&MeshGroup1Data},{&DeinterleavedMeshGroupLoader,&MeshGroup2Data}},
            {{&ImageGroupLoader1,&ImageGroup1Data},{&ImageGroupLoader2,&ImageGroup2Data}}
        );
        /*
        ResourceManager->AddTask(
            {{&InterleavedMeshGroupLoader,&MeshGroup1Data}},
            {{&ImageGroupLoader1,&ImageGroup1Data}}
        );
        
        
        ResourceManager->AddTask(
            {{&DeinterleavedMeshGroupLoader,&MeshGroup2Data}},
            {{&ImageGroupLoader2,&ImageGroup2Data}}
        );
        */
        TaskGraph.Initialize(DemoFramework.Device.get(),DemoFramework.GraphicsQueue,DemoFramework.PresentQueue);
    }
    
    ~lvkFileLoader()
    {
        MeshGroup1Data = {};
        MeshGroup2Data = {};
        
        ImageGroup1Data = {};
        ImageGroup2Data = {};
        
        ResourceManager = nullptr;
        
        vkDeviceWaitIdle(DemoFramework.Device.get());
    }
};

const bool IsResizable = true;

std::unique_ptr<IVKSampleApp> CreateSampleApp(IVKWindow &window)
{
    return std::unique_ptr<IVKSampleApp>(new lvkFileLoader(window));
}
