
#include <lvkSampleApps/lvkDemoUtils.h>

#include <lvkUtils/lvkShaderModuleLibrary.h>

#include <lvkUtils/lvkSmartPtrs/lvkSampler.h>

#include <lvkUtils/lvkSmartPtrs/lvkPipelineLayout.h>
#include <lvkUtils/lvkSmartPtrs/lvkDescriptorSetLayout.h>

#include <lvkUtils/lvkDescriptorPoolUtils.h>

#include "lvkResourceTestRenderer.h"

#include "lvkResourceTestUtils.h"

class lvkResourcePoolApp : public IVKSampleApp
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
    
    lvkPipeline Pipeline;
    
    lvkTestUniformBuffer TestUniformBuffer;
    
    /*
     * Pools
     */
    
    lvkMemoryUtility MemoryUtility;
    
    std::unique_ptr<lvkBufferPool> VertexBufferPool;
    std::unique_ptr<lvkBufferPool> IndexBufferPool;
    
    std::unique_ptr<lvkImagePool> TexturePool;
    
    std::unique_ptr<lvkPMappedBufferPool> StagingBuffer;
    
    /*
     * Renderer
     */
    
    lvkTestRenderer TestRenderer;
    
    // Loaded data
    
    lvkMeshGroupData MeshGroup1Data;
    lvkMeshGroupData MeshGroup2Data;
    
    lvkImageGroupData ImageGroup1Data;
    lvkImageGroupData ImageGroup2Data;
    
    
    std::unique_ptr<lvkResourceTransferBatch> ResourceTransferBatch;
    
    lvkCommandPool    TransferCommandPool;
    
    void UploadResources()
    {
        /*
         * Test resource definitions
         */
        
        TestImageGroup ImageGroup1({{
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
        }});
        
        TestImageGroup ImageGroup2({{
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
        }});
        
        TestMeshGroup MeshGroup1(
            {{-0.125,-0.125,-0.125,0.125,0.125,0.125,0.125,-0.125,0.2,0.0,0.4,0.0,0.4,0.2}},
            {{0.0,0.0,0.0,1.0,1.0,1.0,1.0,0.0,0.0,0.0,0.0,1.0,1.0,1.0}},
            {{0,1,2,0,3,2}}
        );
        
        TestMeshGroup MeshGroup2(
            {{-0.0125,-0.0125,-0.0125,0.0125,0.25,0.125,0.125,-0.25,0.2,0.0,0.4,0.0,0.4,0.2}},
            {{0.0,0.0,0.0,1.0,1.0,1.0,1.0,0.0,0.0,0.0,0.0,1.0,1.0,1.0}},
            {{0,1,2,0,3,2}}
        );
        
        /*
         * Resource manager code
         */
        
        ResourceTransferBatch = std::unique_ptr<lvkResourceTransferBatch>(
            new lvkResourceTransferBatch
        );
        
        ResourceTransferBatch->ImageGroupLoaders.reserve(2);
        ResourceTransferBatch->MeshGroupLoaders.reserve(2);
        
        ResourceTransferBatch->ImageGroupLoaders.emplace_back(ImageGroup1,ImageGroup1Data);
        ResourceTransferBatch->ImageGroupLoaders.emplace_back(ImageGroup2,ImageGroup2Data);
        
        ResourceTransferBatch->MeshGroupLoaders.emplace_back(MeshGroup1,MeshGroup1Data);
        ResourceTransferBatch->MeshGroupLoaders.emplace_back(MeshGroup2,MeshGroup2Data);
        
        lvkImageGroupLoader &ImageGroup1Loader = ResourceTransferBatch->ImageGroupLoaders[0];
        lvkImageGroupLoader &ImageGroup2Loader = ResourceTransferBatch->ImageGroupLoaders[1];
        
        lvkMeshGroupLoader  &MeshGroup1Loader = ResourceTransferBatch->MeshGroupLoaders[0];
        lvkMeshGroupLoader  &MeshGroup2Loader = ResourceTransferBatch->MeshGroupLoaders[1];
        
        // Lets mess up the suballocator memories a bit for testing purposes!
        lrmBufferSuballocGuard TestBufferSuballoc1 = VertexBufferPool->Suballocator.Allocate(1,MeshGroup1.GetVertexDataSize());
        lrmBufferSuballocGuard TestBufferSuballoc2 = IndexBufferPool->Suballocator.Allocate(1,MeshGroup1.GetIndexDataSize());
        
        lrmImageSuballocGuard TestSuballoc1 = TexturePool->Suballocator.Allocate(1);
        lrmImageSuballocGuard TestSuballoc2 = TexturePool->Suballocator.Allocate(1);
        TestSuballoc1 = {};
        
        /*
         * Requesting resources
         */
        
        ImageGroup1Loader.RequestImageMemory(*TexturePool);
        
        for(std::size_t i=0;i < ImageGroup1Loader.ImageGroupLoader.ImageRegions.size();i++)
        {
            ImageGroup1Loader.RequestStagingMemory(i,*StagingBuffer);
        }
        
        // Keep messing it up!
        TestSuballoc2 = {};
        
        /*
         * Requesting resources
         */
        
        ImageGroup2Loader.RequestImageMemory(*TexturePool);
        
        for(std::size_t i=0;i < ImageGroup2Loader.ImageGroupLoader.ImageRegions.size();i++)
        {
            ImageGroup2Loader.RequestStagingMemory(i,*StagingBuffer);
        }
        
        // Keep messing it up!
        lrmBufferSuballocGuard TestBufferSuballoc3 = StagingBuffer->Suballocator.Allocate(1,MeshGroup1.GetVertexDataSize());
        lrmBufferSuballocGuard TestBufferSuballoc4 = StagingBuffer->Suballocator.Allocate(1,MeshGroup1.GetIndexDataSize());
        
        /*
         * Requesting resources
         */
        
        MeshGroup1Loader.RequestVertexBuffer(*VertexBufferPool);
        MeshGroup1Loader.RequestVertexStagingBuffer(*StagingBuffer);
        
        MeshGroup1Loader.RequestIndexBuffer(*IndexBufferPool);
        MeshGroup1Loader.RequestIndexStagingBuffer(*StagingBuffer);
        
        MeshGroup2Loader.RequestVertexBuffer(*VertexBufferPool);
        MeshGroup2Loader.RequestVertexStagingBuffer(*StagingBuffer);
        
        MeshGroup2Loader.RequestIndexBuffer(*IndexBufferPool);
        MeshGroup2Loader.RequestIndexStagingBuffer(*StagingBuffer);
        
        
        /*
         * Loading into staging buffer
         */
        
        
        MeshGroup1Loader.LoadVertexData();
        MeshGroup1Loader.LoadIndexData();
        
        MeshGroup2Loader.LoadVertexData();
        MeshGroup2Loader.LoadIndexData();
        
        ImageGroup1Loader.LoadRegion();
        ImageGroup2Loader.LoadRegion();
        
        TransferCommandPool = lvkCommandPoolUtil::CreateCommandPool(DemoFramework.Device.get(),DemoFramework.GraphicsQueueFamilyIndex,VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
        
        {
            lvkTransferCommandRecorder TransferCommandRecorder;
            
            TransferCommandRecorder.AddMeshGroup(MeshGroup1Loader);
            TransferCommandRecorder.AddMeshGroup(MeshGroup2Loader);
            
            TransferCommandRecorder.AddImageGroup(ImageGroup1Loader);
            TransferCommandRecorder.AddImageGroup(ImageGroup2Loader);
            
            lvkCommandPoolUtil TransferCommandPoolUtil(DemoFramework.Device.get(),TransferCommandPool.get());
            
            ResourceTransferBatch->TransferBuffer = TransferCommandPoolUtil.Allocate();
            TransferCommandRecorder.RecordTransferBuffer(ResourceTransferBatch->TransferBuffer);
            
        }
        TransferTaskGraph.Submit(1,&ResourceTransferBatch->TransferBuffer);
    }
    
    std::vector<VkCommandBuffer> CommandBuffers;
    std::unique_ptr<lvkDemoCommandPool> CommandPoolResetObserver;
    std::vector<std::unique_ptr<lvkDemoBufferRecorder> > CommandRecorders;
    
    lvkDemoTaskGraph TaskGraph;
    
    lvkDemoSingleQueueTransferGraph TransferTaskGraph;
    
public:
    
    virtual void Loop() override
    {
        if(vkGetFenceStatus(DemoFramework.Device.get(),TransferTaskGraph.TransferFence.get()) == VK_SUCCESS)
        {
            TransferCommandPool = nullptr;
            
            if(StagingBuffer != nullptr)
                {std::cout << "Allocations used from staging buffer" << StagingBuffer->Suballocator.GetAllocationList().size() << std::endl;}
            
            ResourceTransferBatch = nullptr;
            
            if(StagingBuffer != nullptr)
                {std::cout << "Allocations used after freeing tmp resources" << StagingBuffer->Suballocator.GetAllocationList().size() << std::endl;}
            
            StagingBuffer = nullptr;
        }
        
        TaskGraph.Submit(DemoFramework.Device.get(),*Swapchain.get(),CommandBuffers.data());
    }
    
    lvkResourcePoolApp(IVKWindow &window)
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
            
            Pipeline = lvkPipeline{NewPipeline,{DemoFramework.Device.get()}};
        }
        
        TransferTaskGraph.Initialize(DemoFramework.Device.get(),DemoFramework.GraphicsQueue);
        
        MemoryUtility = {DemoFramework.Device.get(),DemoFramework.PhysicalDevice};
        
        constexpr static VkDeviceSize BUFFER_POOL_SIZE = 65536;
        
        constexpr static std::uint32_t TEXTURE_POOL_WIDTH  = 2;
        constexpr static std::uint32_t TEXTURE_POOL_HEIGHT = 2;
        constexpr static std::uint32_t TEXTURE_POOL_SIZE   = 8;
        constexpr static VkFormat      TEXTURE_POOL_FORMAT = VK_FORMAT_R32G32B32A32_SFLOAT;
        
        {
            
            std::unique_ptr<lvkMemoryAllocProcElement> DevLocalAllocator = MemoryUtility.CreateAllocator(LVK_STATIC_MEMORY_PROPERTIES);
            std::unique_ptr<lvkMemoryAllocProcElement> FallbackAllocator = MemoryUtility.CreateAllocator(LVK_FALLBACK_MEMORY_PROPERTIES);
            DevLocalAllocator->SetSuccessor(std::move(FallbackAllocator));
            
            lvkResourceFactory BufferFactory(
                DemoFramework.Device.get(),
                std::move(DevLocalAllocator)
            );
            
            /*
             * Allocating buffers
             */
            
            VertexBufferPool = std::unique_ptr<lvkBufferPool>(
                new lvkBufferPool(
                    BufferFactory.CreateBuffer(
                        BUFFER_POOL_SIZE,
                        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
                    ),
                    BUFFER_POOL_SIZE
                )
            );
            
            IndexBufferPool  = std::unique_ptr<lvkBufferPool>(
                new lvkBufferPool(
                    BufferFactory.CreateBuffer(
                        BUFFER_POOL_SIZE,
                        VK_BUFFER_USAGE_INDEX_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_DST_BIT
                    ),
                    BUFFER_POOL_SIZE
                )
            );
            
            /*
             * Allocating textures
             */
            
            TexturePool  = std::unique_ptr<lvkImagePool>(
                new lvkImagePool(
                    BufferFactory.CreateImage(
                        TEXTURE_POOL_WIDTH,
                        TEXTURE_POOL_HEIGHT,
                        TEXTURE_POOL_SIZE,
                        TEXTURE_POOL_FORMAT,
                        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
                    ),
                    TEXTURE_POOL_SIZE
                )
            );
            
            /*
             * Allocating staging buffer
             */
            
            std::unique_ptr<lvkMemoryAllocProcElement> TransferAllocator = MemoryUtility.CreateAllocator(LVK_TRANSFER_MEMORY_PROPERTIES);
            
            lvkResourceFactory TransferBufferFactory(
                DemoFramework.Device.get(),
                std::move(TransferAllocator)
            );
            
            StagingBuffer = std::unique_ptr<lvkPMappedBufferPool>(
                new lvkPMappedBufferPool(
                    TransferBufferFactory.CreateBuffer(
                        BUFFER_POOL_SIZE,
                        VK_BUFFER_USAGE_TRANSFER_SRC_BIT
                    ),
                    BUFFER_POOL_SIZE
                )
            );
        }
        UploadResources();
        
        TestUniformBuffer.Initialize(
            DemoFramework.Device.get(),
            DemoFramework.PhysicalDevice,
            sizeof(TriData),
            NUM_MODELS * 2                   
        );
        
        {
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
                1
            );
            
            {
                lvkTextureDescriptorWriteBatch TextureWriteBatch(DemoFramework.Device.get(),1);
                
                TestRenderer.TexturePoolDescriptorSets.push_back({
                    .ImagePool = TexturePool.get(),
                    .PoolView = TexturePool->ImagePoolResource.CreateImageView(DemoFramework.Device.get(),TEXTURE_POOL_FORMAT),
                    .PoolDescriptorSet = TestRenderer.Descriptors.TextureDescriptorSets[0]
                });
                
                auto &CurrentPool = TestRenderer.TexturePoolDescriptorSets.back();
                
                TextureWriteBatch.SetDescriptorWrite(0,CurrentPool.PoolView.get(),CurrentPool.PoolDescriptorSet);
            }
            
            TestUniformBuffer.WriteDescriptorSet(
                DemoFramework.Device.get(),
                TestRenderer.Descriptors.UboDescriptorSet
            );
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
                        
                        vkCmdBindPipeline(CommandBuffers[i],VK_PIPELINE_BIND_POINT_GRAPHICS,Pipeline.get());
                        
                        TestRenderer.BindMeshGroup(CommandBuffers[i],MeshGroup1Data);
                        TestRenderer.BindImageGroup(CommandBuffers[i],ImageGroup1Data);
                        
                        for(int j=0;j < NUM_MODELS*2;j++)
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
                        TestRenderer.BindImageGroup(CommandBuffers[i],ImageGroup2Data);
                        
                        for(int j=0;j < NUM_MODELS*2;j++)
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
        
        TaskGraph.Initialize(DemoFramework.Device.get(),DemoFramework.GraphicsQueue,DemoFramework.PresentQueue);
    }
    
    ~lvkResourcePoolApp()
    {
        vkDeviceWaitIdle(DemoFramework.Device.get());
    }
};

const bool IsResizable = true;

std::unique_ptr<IVKSampleApp> CreateSampleApp(IVKWindow &window)
{
    return std::unique_ptr<IVKSampleApp>(new lvkResourcePoolApp(window));
}
