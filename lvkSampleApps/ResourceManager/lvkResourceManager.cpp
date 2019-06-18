
#include <lvkSampleApps/lvkDemoUtils.h>

#include <lvkUtils/lvkShaderModuleLibrary.h>

#include <lvkUtils/lvkSmartPtrs/lvkSampler.h>

#include <lvkUtils/lvkSmartPtrs/lvkPipelineLayout.h>
#include <lvkUtils/lvkSmartPtrs/lvkDescriptorSetLayout.h>

#include <lvkUtils/lvkDescriptorPoolUtils.h>


#include "lvkResourceTestRenderer.h"

#include "lvkResourceTestUtils.h"


class lvkResourceManagerApp : public IVKSampleApp
{
private:
    
    struct TriData
    {
        float x;float y;float z;float w;
    };
    
    constexpr static std::size_t NUM_MODELS = 5;
    
    lvkDemoFramework DemoFramework;
    
    lvkShaderModuleLibrary ShaderModuleLibrary;
    
    std::unique_ptr<lvkSwapchain> Swapchain;
    lvkRenderPass RenderPass;
    std::unique_ptr<lvkDemoSwapchainFramebuffers> DemoSwapchainFramebuffers;
    
    lvkPipeline DeinterleavedPipelineNrm;
    
    lvkTestUniformBuffer TestUniformBuffer;
    
    /*
     * Pools
     */
    
    lvkTestPools TestPools;
    
    // Loaded data
    
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
    
    TestImageGroup ImageGroup1 = {{{
        {
            0.0,0.0,0.0,1.0, 1.0,1.0,1.0,1.0,
            1.0,1.0,1.0,1.0, 0.0,0.0,0.0,1.0,
        },
        {
            1.0,1.0,0.0,1.0, 0.0,1.0,1.0,1.0,
            0.0,1.0,1.0,1.0, 1.0,1.0,0.0,1.0,
        },
        {
            1.0,0.0,1.0,1.0, 0.0,0.0,1.0,1.0,
            0.0,0.0,1.0,1.0, 1.0,0.0,1.0,1.0,
        },
        {
            0.0,1.0,1.0,1.0, 0.0,1.0,0.0,1.0,
            0.0,1.0,0.0,1.0, 0.0,1.0,1.0,1.0,
        },
    }}};
    
    TestImageGroup ImageGroup2 = {{{
        {
            0.0,0.0,0.0,1.0, 0.0,1.0,1.0,1.0,
            1.0,1.0,1.0,1.0, 0.0,0.0,0.0,1.0,
        },
        {
            1.0,1.0,0.0,1.0, 0.0,1.0,1.0,1.0,
            0.0,1.0,0.0,1.0, 1.0,1.0,0.0,1.0,
        },
        {
            1.0,0.0,1.0,1.0, 0.0,0.0,0.0,1.0,
            0.0,0.0,1.0,1.0, 1.0,0.0,1.0,1.0,
        },
        {
            0.0,1.0,1.0,1.0, 0.0,1.0,0.0,1.0,
            0.0,1.0,0.0,1.0, 0.0,0.0,1.0,1.0,
        }
    }}};
    
    TestMeshGroup MeshGroup1 = {
        {{-0.125,-0.125,-0.125,0.125,0.125,0.125,0.125,-0.125,0.2,0.0,0.4,0.0,0.4,0.2}},
        {{0.0,0.0,0.0,1.0,1.0,1.0,1.0,0.0,0.0,0.0,0.0,1.0,1.0,1.0}},
        {{0,1,2,0,3,2}}
    };
    
    TestMeshGroup MeshGroup2 = {
        {{-0.0125,-0.0125,-0.0125,0.0125,0.25,0.125,0.125,-0.25,0.2,0.0,0.4,0.0,0.4,0.2}},
        {{0.0,0.0,0.0,1.0,1.0,1.0,1.0,0.0,0.0,0.0,0.0,1.0,1.0,1.0}},
        {{0,1,2,0,3,2}}
    };
    
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
        
        if(!CmdBuffersAllocated && ResourcesLoaded())
        {
            CmdBuffersAllocated = true;
            
            TestUniformBuffer.Initialize(
                DemoFramework.Device.get(),
                DemoFramework.PhysicalDevice,
                sizeof(TriData),
                NUM_MODELS * 2
            );
            
            {
                //lvkMappedMemory MappedMemory = UniformBufferResource.Allocation.Map();
                lMemoryView FullMemoryView = TestUniformBuffer.MappedMemory.GetView();
                
                lBlockMemoryView TriDataBufferView((char*)FullMemoryView.GetPtr(),NUM_MODELS * 2,TestUniformBuffer.StructSize);
                
                for(int i=0;i < NUM_MODELS;i++)
                {
                    TriData *TriDataElement = (TriData *)TriDataBufferView.GetBlock(i);
                    TriDataElement->x =  -0.8 + i*0.4;
                    TriDataElement->y =   0.8 - i*0.4;
                    TriDataElement->w =  ImageGroup1Data.ImageSuballocs.GetTextureAllocs()[i % ImageGroup1Data.ImageSuballocs.GetTextureAllocs().size()];
                }
                
                for(int i=0;i < NUM_MODELS;i++)
                {
                    TriData *TriDataElement = (TriData *)TriDataBufferView.GetBlock(NUM_MODELS + i);
                    TriDataElement->x =  -0.8 + i*0.2;
                    TriDataElement->y =   1.0 - 0.8 - i*0.2;
                    TriDataElement->w =  ImageGroup2Data.ImageSuballocs.GetTextureAllocs()[i % ImageGroup2Data.ImageSuballocs.GetTextureAllocs().size()];
                }
            }
            
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
                                Swapchain->GetExtent()
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
                            
                            vkCmdBindPipeline(CommandBuffers[i],VK_PIPELINE_BIND_POINT_GRAPHICS,DeinterleavedPipelineNrm.get());
                            
                            TestRenderer.BindMeshGroup(CommandBuffers[i],MeshGroup1Data);
                            
                            TestRenderer.BindImageGroup(CommandBuffers[i],ImageGroup1Data);
                            
                            for(int j=0;j < NUM_MODELS;j++)
                            {
                                TestUniformBuffer.BindBlock(
                                    CommandBuffers[i],
                                    TestRenderer.TestPipelineLayout.PipelineLayout.get(),
                                    j
                                );
                                
                                vkCmdDrawIndexed(
                                    CommandBuffers[i],
                                    6,
                                    1,
                                    0,
                                    0,
                                    0
                                );
                                
                                vkCmdDraw(
                                    CommandBuffers[i],
                                    3,
                                    1,
                                    4,
                                    0
                                );
                            }
                            
                            TestRenderer.BindImageGroup(CommandBuffers[i],ImageGroup2Data);
                            
                            for(int j=NUM_MODELS;j < NUM_MODELS*2;j++)
                            {
                                TestUniformBuffer.BindBlock(
                                    CommandBuffers[i],
                                    TestRenderer.TestPipelineLayout.PipelineLayout.get(),
                                    j
                                );
                                
                                vkCmdDrawIndexed(
                                    CommandBuffers[i],
                                    6,
                                    1,
                                    0,
                                    0,
                                    0
                                );
                                
                                vkCmdDraw(
                                    CommandBuffers[i],
                                    3,
                                    1,
                                    4,
                                    0
                                );
                            }
                        
                        
                            TestRenderer.BindMeshGroup(CommandBuffers[i],MeshGroup2Data);
                            
                            TestRenderer.BindImageGroup(CommandBuffers[i],ImageGroup1Data);
                            
                            for(int j=0;j < NUM_MODELS;j++)
                            {
                                TestUniformBuffer.BindBlock(
                                    CommandBuffers[i],
                                    TestRenderer.TestPipelineLayout.PipelineLayout.get(),
                                    j
                                );
                                
                                vkCmdDrawIndexed(
                                    CommandBuffers[i],
                                    6,
                                    1,
                                    0,
                                    0,
                                    0
                                );
                                
                                vkCmdDraw(
                                    CommandBuffers[i],
                                    3,
                                    1,
                                    4,
                                    0
                                );
                            }
                            
                            TestRenderer.BindImageGroup(CommandBuffers[i],ImageGroup2Data);
                            
                            for(int j=NUM_MODELS;j < NUM_MODELS*2;j++)
                            {
                                TestUniformBuffer.BindBlock(
                                    CommandBuffers[i],
                                    TestRenderer.TestPipelineLayout.PipelineLayout.get(),
                                    j
                                );
                                
                                vkCmdDrawIndexed(
                                    CommandBuffers[i],
                                    6,
                                    1,
                                    0,
                                    0,
                                    0
                                );
                                
                                vkCmdDraw(
                                    CommandBuffers[i],
                                    3,
                                    1,
                                    4,
                                    0
                                );
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
    
    lvkResourceManagerApp(IVKWindow &window)
        :DemoFramework(window)
    {
        ShaderModuleLibrary = {DemoFramework.Device.get()};
        
        Swapchain = std::unique_ptr<lvkSwapchain>(new lvkSwapchain(DemoFramework.Device.get(),DemoFramework.PhysicalDevice,DemoFramework.Surface.get()));
        RenderPass = DemoFramework.CreateSimpleRenderPass(DemoFramework.Device.get(),Swapchain->GetFormat());
        DemoSwapchainFramebuffers = std::unique_ptr<lvkDemoSwapchainFramebuffers>(new lvkDemoSwapchainFramebuffers(DemoFramework.Device.get(),RenderPass.get(),*Swapchain.get()));
        
        TestRenderer.TestPipelineLayout.Initialize(DemoFramework.Device.get());
        
        {
            lvkDemoShaderStages           DemoShaderStages;
            lvkPipelineFixedFunctionState FixedFunctionState;
            lvkPipelineDynamicState       DynamicState;
            lvkDemoColorBlendAttachments  DemoColorBlendAttachments;
            VkGraphicsPipelineCreateInfo  PipelineCreateInfo = LVK_EMPTY_PIPELINE_CREATE_INFO;
            
            
            lvkVertexInputDescription VertexInputDescription;
            
            VertexInputDescription.AddVertexInputBinding(0,2*sizeof(float),VK_VERTEX_INPUT_RATE_VERTEX);
            VertexInputDescription.AddVertexInputBinding(1,2*sizeof(float),VK_VERTEX_INPUT_RATE_VERTEX);
            VertexInputDescription.AddVertexInputAttributeDescription(0,0,VK_FORMAT_R32G32_SFLOAT,0);
            VertexInputDescription.AddVertexInputAttributeDescription(1,1,VK_FORMAT_R32G32_SFLOAT,0);
            
            DemoShaderStages.SetShaders(
                ShaderModuleLibrary.GetShaderModule("../Shaders/TexVertexShader.spv"),
                ShaderModuleLibrary.GetShaderModule("../Shaders/TexFragmentShader.spv")
            );
            
            DynamicState.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
            DynamicState.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR);
            
            PipelineCreateInfo.renderPass = RenderPass.get();
            PipelineCreateInfo.subpass = 0;
            PipelineCreateInfo.layout = TestRenderer.TestPipelineLayout.PipelineLayout.get();
            
            VertexInputDescription.AssignToPipelineCreateInfo(PipelineCreateInfo);
            DemoShaderStages.AssignToPipelineCreateInfo(PipelineCreateInfo);
            FixedFunctionState.AssignToPipelineCreateInfo(PipelineCreateInfo);
            DynamicState.AssignToPipelineCreateInfo(PipelineCreateInfo);
            DemoColorBlendAttachments.AssignToPipelineCreateInfo(PipelineCreateInfo);
            
            std::cout << "Creating graphics pipeline" << std::endl;
            VkPipeline NewPipeline;
            if(vkCreateGraphicsPipelines(DemoFramework.Device.get(),VK_NULL_HANDLE,1,&PipelineCreateInfo,nullptr,&NewPipeline) != VK_SUCCESS)
            {
                std::cout << "Error while creating pipeline" << std::endl;
            }
            
            DeinterleavedPipelineNrm = lvkPipeline{NewPipeline,{DemoFramework.Device.get()}};
        }
        
        TestPools.Initialize(
            DemoFramework.Device.get(),
            DemoFramework.PhysicalDevice,
            {
                .MeshBufferPoolSize    = 65536,
                .StagingBufferPoolSize = 65536,
                .TextureWidth  = 2,
                .TextureHeight = 2,
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
            {{&MeshGroup1,&MeshGroup1Data},{&MeshGroup2,&MeshGroup2Data}},
            {{&ImageGroup1,&ImageGroup1Data},{&ImageGroup2,&ImageGroup2Data}}
        );
        /*
        ResourceManager->AddTask(
            {{&MeshGroup1,&MeshGroup1Data}},
            {{&ImageGroup1,&ImageGroup1Data}}
        );
        
        
        ResourceManager->AddTask(
            {{&MeshGroup2,&MeshGroup2Data}},
            {{&ImageGroup2,&ImageGroup2Data}}
        );
        */
        TaskGraph.Initialize(DemoFramework.Device.get(),DemoFramework.GraphicsQueue,DemoFramework.PresentQueue);
    }
    
    ~lvkResourceManagerApp()
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
    return std::unique_ptr<IVKSampleApp>(new lvkResourceManagerApp(window));
}
