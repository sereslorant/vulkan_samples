
#include <lvkSampleApps/lvkDemoUtils.h>

#include <lvkUtils/lvkShaderModuleLibrary.h>

#include <lvkUtils/lvkSmartPtrs/lvkSampler.h>

#include <lvkUtils/lvkSmartPtrs/lvkPipelineLayout.h>
#include <lvkUtils/lvkSmartPtrs/lvkDescriptorSetLayout.h>

#include <lvkUtils/lvkDescriptorPoolUtils.h>


class lvkTextureArrayApp : public IVKSampleApp
{
private:
    
    struct TriData
    {
        float x;float y;float z;float w;
    };
    
    std::size_t NumTriangles = 5;
    std::size_t StructSize = 0;
    
    
    lvkDemoFramework DemoFramework;
    
    lvkShaderModuleLibrary ShaderModuleLibrary;
    
    std::unique_ptr<lvkSwapchain> Swapchain;
    lvkRenderPass RenderPass;
    std::unique_ptr<lvkDemoSwapchainFramebuffers> DemoSwapchainFramebuffers;
    
    lvkDescriptorSetLayout UniformBufferLayout;
    lvkDescriptorSetLayout TextureArrayLayout;
    lvkPipelineLayout      PipelineLayout;
    
    lvkPipeline Pipeline;
    
    lvkBufferResource UniformBufferResource;
    
    lvkImageResource TextureArrayResource;
    lvkImageView     TextureImgView;
    lvkSampler       TextureImgSampler;
    
    lvkDescriptorPool DescriptorPool;
    
    std::vector<VkDescriptorSet> UboDescriptorSets;
    std::vector<VkDescriptorSet> TexDescriptorSets;
    
    std::vector<VkCommandBuffer> CommandBuffers;
    std::unique_ptr<lvkDemoCommandPool> CommandPoolResetObserver;
    std::vector<std::unique_ptr<lvkDemoBufferRecorder> > CommandRecorders;
    
    lvkDemoTaskGraph TaskGraph;
    
    lvkBufferResource StagingBufferResource;
    lvkCommandPool    TransferCommandPool;
    
    lvkDemoSingleQueueTransferGraph TransferTaskGraph;
    
public:
    
    virtual void Loop() override
    {
        if(vkGetFenceStatus(DemoFramework.Device.get(),TransferTaskGraph.TransferFence.get()) == VK_SUCCESS)
        {
            TransferCommandPool = nullptr;
            StagingBufferResource = {};
        }
        
        TaskGraph.Submit(DemoFramework.Device.get(),*Swapchain.get(),CommandBuffers.data());
    }
    
    lvkTextureArrayApp(IVKWindow &window)
        :DemoFramework(window)
    {
        ShaderModuleLibrary = {DemoFramework.Device.get()};
        
        Swapchain = std::unique_ptr<lvkSwapchain>(new lvkSwapchain(DemoFramework.Device.get(),DemoFramework.PhysicalDevice,DemoFramework.Surface.get()));
        RenderPass = DemoFramework.CreateSimpleRenderPass(DemoFramework.Device.get(),Swapchain->GetFormat());
        DemoSwapchainFramebuffers = std::unique_ptr<lvkDemoSwapchainFramebuffers>(new lvkDemoSwapchainFramebuffers(DemoFramework.Device.get(),RenderPass.get(),*Swapchain.get()));
        
        {
            {
                VkSamplerCreateInfo SamplerCreateInfo = LVK_LINEAR_CLAMPED_SAMPLER_CREATE_INFO;
                
                VkSampler Sampler = VK_NULL_HANDLE;
                if(vkCreateSampler(DemoFramework.Device.get(),&SamplerCreateInfo,nullptr,&Sampler) != VK_SUCCESS)
                {
                    std::cout << "Error while creating sampler" << std::endl;
                }
                
                TextureImgSampler = lvkSampler{Sampler,{DemoFramework.Device.get()}};
            }
            
            {
                VkDescriptorSetLayoutBinding UboLayoutBinding = {};
                UboLayoutBinding.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                UboLayoutBinding.descriptorCount = 1;
                UboLayoutBinding.binding = 0;
                
                UboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
                
                VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo = {};
                DescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                DescriptorSetLayoutCreateInfo.bindingCount = 1;
                DescriptorSetLayoutCreateInfo.pBindings = &UboLayoutBinding;
                
                std::cout << "Creating descriptor set layout" << std::endl;
                VkDescriptorSetLayout TmpDescriptorSetLayout = nullptr;
                if(vkCreateDescriptorSetLayout(DemoFramework.Device.get(), &DescriptorSetLayoutCreateInfo, nullptr, &TmpDescriptorSetLayout) != VK_SUCCESS)
                {
                    std::cout << "Error while creating descriptor set layout" << std::endl;
                }
                
                UniformBufferLayout = lvkDescriptorSetLayout{TmpDescriptorSetLayout,{DemoFramework.Device.get()}};
            }
            
            {
                std::array<VkSampler,1> Samplers = {TextureImgSampler.get()};
                
                VkDescriptorSetLayoutBinding TexLayoutBinding = {};
                TexLayoutBinding.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                TexLayoutBinding.descriptorCount = 1;
                TexLayoutBinding.binding = 1;
                TexLayoutBinding.pImmutableSamplers = Samplers.data();
                
                TexLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                
                VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo = {};
                DescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                DescriptorSetLayoutCreateInfo.bindingCount = 1;
                DescriptorSetLayoutCreateInfo.pBindings = &TexLayoutBinding;
                
                std::cout << "Creating descriptor set layout" << std::endl;
                VkDescriptorSetLayout TmpDescriptorSetLayout = nullptr;
                if(vkCreateDescriptorSetLayout(DemoFramework.Device.get(), &DescriptorSetLayoutCreateInfo, nullptr, &TmpDescriptorSetLayout) != VK_SUCCESS)
                {
                    std::cout << "Error while creating descriptor set layout" << std::endl;
                }
                
                TextureArrayLayout = lvkDescriptorSetLayout{TmpDescriptorSetLayout,{DemoFramework.Device.get()}};
            }
            
            VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo = LVK_EMPTY_PIPELINE_LAYOUT_CREATE_INFO;
            
            std::array<VkDescriptorSetLayout,2> DescriptorSetLayouts = {UniformBufferLayout.get(),TextureArrayLayout.get()};
            
            PipelineLayoutCreateInfo.setLayoutCount = DescriptorSetLayouts.size();
            PipelineLayoutCreateInfo.pSetLayouts    = DescriptorSetLayouts.data();
            
            std::cout << "Creating pipeline layout" << std::endl;
            VkPipelineLayout NewPipelineLayout;
            if(vkCreatePipelineLayout(DemoFramework.Device.get(),&PipelineLayoutCreateInfo,nullptr,&NewPipelineLayout) != VK_SUCCESS)
            {
                std::cout << "Error while creating pipeline layout" << std::endl;
            }
            
            PipelineLayout = lvkPipelineLayout{NewPipelineLayout,{DemoFramework.Device.get()}};
        }
        
        {
            lvkDemoShaderStages           DemoShaderStages;
            lvkPipelineFixedFunctionState FixedFunctionState;
            lvkPipelineDynamicState       DynamicState;
            lvkDemoColorBlendAttachments  DemoColorBlendAttachments;
            VkGraphicsPipelineCreateInfo  PipelineCreateInfo = LVK_EMPTY_PIPELINE_CREATE_INFO;
            
            DemoShaderStages.SetShaders(
                ShaderModuleLibrary.GetShaderModule("../Shaders/TexHardcodedVertexShader.spv"),
                ShaderModuleLibrary.GetShaderModule("../Shaders/TexFragmentShader.spv")
            );
            
            DynamicState.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
            DynamicState.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR);
            
            PipelineCreateInfo.renderPass = RenderPass.get();
            PipelineCreateInfo.subpass = 0;
            PipelineCreateInfo.layout = PipelineLayout.get();
            
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
        
        lvkMemoryUtility MemoryUtility(DemoFramework.Device.get(),DemoFramework.PhysicalDevice);
        std::unique_ptr<lvkMemoryAllocProcElement> Allocator = MemoryUtility.CreateAllocator(LVK_TRANSFER_MEMORY_PROPERTIES);
        
        lvkResourceFactory BufferFactory(
            DemoFramework.Device.get(),
            std::move(Allocator)
        );
        
        VkPhysicalDeviceProperties DeviceProperties;
        vkGetPhysicalDeviceProperties(DemoFramework.PhysicalDevice,&DeviceProperties);
        
        StructSize = lPaddedStructSize(DeviceProperties.limits.minUniformBufferOffsetAlignment,sizeof(TriData));
        
        std::uint32_t UniformBufferSize = NumTriangles * StructSize;
        UniformBufferResource = BufferFactory.CreateBuffer(UniformBufferSize,VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        
        float Images[4][4*4] =
        {
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
        };
        
        std::uint32_t Width = 2;
        std::uint32_t Height = 2;
        
        std::uint32_t ImageCount = 4;
        
        
        {
            lvkMappedMemory MappedMemory = UniformBufferResource.Allocation.Map();
            lMemoryView FullMemoryView = MappedMemory.GetView();
            
            lBlockMemoryView TriDataBufferView((char*)FullMemoryView.GetPtr(),NumTriangles,StructSize);
            
            for(int i=0;i < NumTriangles;i++)
            {
                TriData *TriDataElement = (TriData *)TriDataBufferView.GetBlock(i);
                TriDataElement->x =  -0.8 + i*0.4;
                TriDataElement->y =   0.8 - i*0.4;
                TriDataElement->w =  i % ImageCount;
            }
        }
        
        std::uint64_t SingleImageSize = Width * Height * 4 * sizeof(float);
        std::uint64_t CombinedImageSize = ImageCount * SingleImageSize;
        
        
        TextureArrayResource = BufferFactory.CreateImage(
            Width,
            Height,
            ImageCount,
            VK_FORMAT_R32G32B32A32_SFLOAT,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
        );
        
        TextureImgView = TextureArrayResource.CreateImageView(DemoFramework.Device.get(),VK_FORMAT_R32G32B32A32_SFLOAT);
        
        StagingBufferResource = BufferFactory.CreateBuffer(CombinedImageSize,VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        {
            lvkMappedMemory MappedMemory = StagingBufferResource.Allocation.Map();
            lMemoryView FullMemoryView   = MappedMemory.GetView();
            
            for(int i=0;i < ImageCount;i++)
            {
                lMemoryView SingleImageView = FullMemoryView.GetView(i*SingleImageSize,SingleImageSize);
                SingleImageView.MemCpySec(Images[i],SingleImageSize);
            }
        }
        
        TransferTaskGraph.Initialize(DemoFramework.Device.get(),DemoFramework.GraphicsQueue);
        {
            lvkBufferToImageTransferRecorder BufferToImageTransferRecorder;
            
            BufferToImageTransferRecorder.ImageTransfers.push_back(
                {
                    .Buffer         = StagingBufferResource.Buffer.get(),
                    .Image          = TextureArrayResource.Image.get(),
                    .Width          = TextureArrayResource.Width,
                    .Height         = TextureArrayResource.Height,
                }
            );
            
            BufferToImageTransferRecorder.ImageTransfers[0].AddTransfer(0,0,TextureArrayResource.ImageCount);
            
            std::vector<VkImageMemoryBarrier> ImageBarriers;
            BufferToImageTransferRecorder.Resize(ImageBarriers);
            
            TransferCommandPool = lvkCommandPoolUtil::CreateCommandPool(DemoFramework.Device.get(),DemoFramework.GraphicsQueueFamilyIndex,VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
            lvkCommandPoolUtil TransferCommandPoolUtil(DemoFramework.Device.get(),TransferCommandPool.get());
            
            VkCommandBuffer TransferBuffer = TransferCommandPoolUtil.Allocate();
            {
                lvkRecordCmdBufferGuard TransferGuard(TransferBuffer,VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
                
                BufferToImageTransferRecorder.RecordTransferBarrier(ImageBarriers,TransferBuffer);
                BufferToImageTransferRecorder.RecordTransferOps(TransferBuffer);
                BufferToImageTransferRecorder.RecordShaderAccessBarrier(ImageBarriers,TransferBuffer);
            }
            
            TransferTaskGraph.Submit(1,&TransferBuffer);
        }
        
        {
            lvkDescriptorPoolSizes     DescriptorPoolSizes;
            VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo = LVK_EMPTY_DESCRIPTOR_POOL_CREATE_INFO;
            
            DescriptorPoolSizes.SetDescriptorPoolSize(
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                Swapchain->GetImageCount()
            );
            
            DescriptorPoolSizes.SetDescriptorPoolSize(
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                Swapchain->GetImageCount()
            );
            
            DescriptorPoolCreateInfo.maxSets = Swapchain->GetImageCount()*2;
            
            DescriptorPoolSizes.AssignToDescriptorPoolCreateInfo(DescriptorPoolCreateInfo);
            
            std::cout << "Creating descriptor pool" << std::endl;
            VkDescriptorPool NewDescriptorPool = VK_NULL_HANDLE;
            if(vkCreateDescriptorPool(DemoFramework.Device.get(),&DescriptorPoolCreateInfo,nullptr,&NewDescriptorPool) != VK_SUCCESS)
            {
                std::cout << "Error while creating descriptor pool" << std::endl;
            }
            DescriptorPool = lvkDescriptorPool{NewDescriptorPool,DemoFramework.Device.get()};
            
            lvkDescriptorPoolUtil DescriptorPoolUtil(DemoFramework.Device.get(),DescriptorPool.get());
            
            UboDescriptorSets = DescriptorPoolUtil.Allocate({Swapchain->GetImageCount(),UniformBufferLayout.get()});
            TexDescriptorSets = DescriptorPoolUtil.Allocate({Swapchain->GetImageCount(),TextureArrayLayout.get()});
            
            for(int i=0;i < Swapchain->GetImageCount();i++)
            {
                VkDescriptorBufferInfo BufferInfo = {};
                BufferInfo.buffer = UniformBufferResource.Buffer.get();
                BufferInfo.offset = 0;
                BufferInfo.range  = StructSize;
                
                VkWriteDescriptorSet UboDescriptorWrite = {};
                UboDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                UboDescriptorWrite.dstSet          = UboDescriptorSets[i];
                UboDescriptorWrite.dstBinding      = 0;
                UboDescriptorWrite.dstArrayElement = 0;
                
                UboDescriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                UboDescriptorWrite.descriptorCount = 1;
                
                UboDescriptorWrite.pBufferInfo = &BufferInfo;
                
                vkUpdateDescriptorSets(DemoFramework.Device.get(),1,&UboDescriptorWrite,0,nullptr);
                
                VkDescriptorImageInfo ImageInfo = {};
                ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                ImageInfo.imageView   = TextureImgView.get();
                //ImageInfo.sampler     = TextureImgSampler.get();
                
                VkWriteDescriptorSet TexDescriptorWrite = {};
                
                TexDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                TexDescriptorWrite.dstSet          = TexDescriptorSets[i];
                TexDescriptorWrite.dstBinding      = 1;
                TexDescriptorWrite.dstArrayElement = 0;
                
                TexDescriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                TexDescriptorWrite.descriptorCount = 1;
                
                TexDescriptorWrite.pImageInfo = &ImageInfo;
                
                vkUpdateDescriptorSets(DemoFramework.Device.get(),1,&TexDescriptorWrite,0,nullptr);
            }
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
                        
                        vkCmdBindDescriptorSets(CommandBuffers[i],VK_PIPELINE_BIND_POINT_GRAPHICS,PipelineLayout.get(),1,1,&TexDescriptorSets[i],0,nullptr);
                        
                        for(int j=0;j < NumTriangles;j++)
                        {
                            uint32_t BufferOffset = j*StructSize;
                            
                            vkCmdBindDescriptorSets(CommandBuffers[i],VK_PIPELINE_BIND_POINT_GRAPHICS,PipelineLayout.get(),0,1,&UboDescriptorSets[i],1,&BufferOffset);
                            
                            vkCmdDraw(CommandBuffers[i],6,1,0,0);
                        }
                    },
                    *Swapchain.get()
                )
            );
        }
        
        TaskGraph.Initialize(DemoFramework.Device.get(),DemoFramework.GraphicsQueue,DemoFramework.PresentQueue);
    }
    
    ~lvkTextureArrayApp()
    {
        vkDeviceWaitIdle(DemoFramework.Device.get());
    }
};

const bool IsResizable = true;

std::unique_ptr<IVKSampleApp> CreateSampleApp(IVKWindow &window)
{
    return std::unique_ptr<IVKSampleApp>(new lvkTextureArrayApp(window));
}
