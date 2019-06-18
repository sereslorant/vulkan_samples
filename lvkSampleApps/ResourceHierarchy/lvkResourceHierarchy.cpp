
#include <lvkSampleApps/lvkDemoUtils.h>

#include <lvkUtils/lvkShaderModuleLibrary.h>

#include <lvkUtils/lvkSmartPtrs/lvkSampler.h>

#include <lvkUtils/lvkSmartPtrs/lvkPipelineLayout.h>
#include <lvkUtils/lvkSmartPtrs/lvkDescriptorSetLayout.h>

#include <lvkUtils/lvkDescriptorPoolUtils.h>

#include <lvkRenderer/lvkResourceHierarchy.h>

class lvkResourceHierarchyApp : public IVKSampleApp
{
private:
    
    struct TriData
    {
        float x;float y;float z;float w;
    };
    
    std::size_t NumTriangles = 5;
    std::uint32_t StructSize = 0;
    
    
    lvkDemoFramework DemoFramework;
    
    lvkShaderModuleLibrary ShaderModuleLibrary;
    
    std::unique_ptr<lvkSwapchain> Swapchain;
    lvkRenderPass RenderPass;
    std::unique_ptr<lvkDemoSwapchainFramebuffers> DemoSwapchainFramebuffers;
    
    lvkSampler             TextureImgSampler;
    lvkDescriptorSetLayout UniformBufferLayout;
    lvkDescriptorSetLayout TextureArrayLayout;
    lvkPipelineLayout      PipelineLayout;
    
    lvkPipeline Pipeline;
    
    lvkBufferResource UniformBufferResource1;
    lvkBufferResource UniformBufferResource2;
    
    lvkImageResource TextureArrayResource1;
    lvkImageView     TextureImgView1;
    
    lvkImageResource TextureArrayResource2;
    lvkImageView     TextureImgView2;
    
    lvkBufferResource VertexBufferResource;
    lvkBufferResource IndexBufferResource;
    
    lvkDescriptorPool DescriptorPool;
    
    lvkDescriptorSet        TexArrayDescriptorSet1;
    lvkDescriptorSet        TexArrayDescriptorSet2;
    lvkDynamicDescriptorSet UniformBufferDescriptorSet1;
    lvkDynamicDescriptorSet UniformBufferDescriptorSet2;
    
    lvkMeshGroupVertexData MeshVertexBuffers;
    lvkMeshGroupIndexData  MeshIndexBuffer;
    lrMeshGroup            MeshGroup;
    
    lrPipelineState PipelineNode;
    
    lrUniformBindings TextureGroup1;
    lrUniformBindings TextureGroup2;
    
    lrMeshGroupBindings MeshGroup1;
    lrMeshGroupBindings MeshGroup2;
    
    lrDynamicUniformBindings UniformSubBuffers1;
    lrDynamicUniformBindings UniformSubBuffers2;
    
    lrResourceNode TwoMeshes;
    
    lrMeshDrawCall Mesh1;
    lrMeshDrawCall Mesh2;
    
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
    
    lvkResourceHierarchyApp(IVKWindow &window)
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
            PipelineCreateInfo.layout = PipelineLayout.get();
            
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
        UniformBufferResource1 = BufferFactory.CreateBuffer(UniformBufferSize,VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        UniformBufferResource2 = BufferFactory.CreateBuffer(UniformBufferSize,VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        
        float Images1[4][4*4] =
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
        
        float Images2[4][4*4] =
        {
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
        };
        
        std::uint32_t Width = 2;
        std::uint32_t Height = 2;
        
        std::uint32_t ImageCount = 4;
        
        
        {
            lvkMappedMemory MappedMemory = UniformBufferResource1.Allocation.Map();
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
        
        {
            lvkMappedMemory MappedMemory = UniformBufferResource2.Allocation.Map();
            lMemoryView FullMemoryView = MappedMemory.GetView();
            
            lBlockMemoryView TriDataBufferView((char*)FullMemoryView.GetPtr(),NumTriangles,StructSize);
            
            for(int i=0;i < NumTriangles;i++)
            {
                TriData *TriDataElement = (TriData *)TriDataBufferView.GetBlock(i);
                TriDataElement->x =  -0.8 + i*0.2;
                TriDataElement->y =   1.0 - 0.8 - i*0.2;
                TriDataElement->w =  i % ImageCount;
            }
        }
        
        std::uint64_t SingleImageSize = Width * Height * 4 * sizeof(float);
        std::uint64_t CombinedImageSize = ImageCount * SingleImageSize;
        
        
        TextureArrayResource1 = BufferFactory.CreateImage(
            Width,
            Height,
            ImageCount,
            VK_FORMAT_R32G32B32A32_SFLOAT,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
        );
        
        TextureImgView1 = TextureArrayResource1.CreateImageView(DemoFramework.Device.get(),VK_FORMAT_R32G32B32A32_SFLOAT);
        
        
        TextureArrayResource2 = BufferFactory.CreateImage(
            Width,
            Height,
            ImageCount,
            VK_FORMAT_R32G32B32A32_SFLOAT,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
        );
        
        TextureImgView2 = TextureArrayResource2.CreateImageView(DemoFramework.Device.get(),VK_FORMAT_R32G32B32A32_SFLOAT);
        
        StagingBufferResource = BufferFactory.CreateBuffer(CombinedImageSize*2,VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        {
            lvkMappedMemory MappedMemory = StagingBufferResource.Allocation.Map();
            lMemoryView FullMemoryView   = MappedMemory.GetView();
            
            for(int i=0;i < ImageCount;i++)
            {
                lMemoryView SingleImageView = FullMemoryView.GetView(i*SingleImageSize,SingleImageSize);
                SingleImageView.MemCpySec(Images1[i],SingleImageSize);
            }
            
            for(int i=0;i < ImageCount;i++)
            {
                lMemoryView SingleImageView = FullMemoryView.GetView((ImageCount + i)*SingleImageSize,SingleImageSize);
                SingleImageView.MemCpySec(Images2[i],SingleImageSize);
            }
        }
        
        TransferTaskGraph.Initialize(DemoFramework.Device.get(),DemoFramework.GraphicsQueue);
        {
            lvkBufferToImageTransferRecorder BufferToImageTransferRecorder;
            
            BufferToImageTransferRecorder.ImageTransfers.push_back(
                {
                    .Buffer         = StagingBufferResource.Buffer.get(),
                    .Image          = TextureArrayResource1.Image.get(),
                    .Width          = TextureArrayResource1.Width,
                    .Height         = TextureArrayResource1.Height,
                }
            );
            
            BufferToImageTransferRecorder.ImageTransfers[0].AddTransfer(0,0,TextureArrayResource1.ImageCount);
            
            BufferToImageTransferRecorder.ImageTransfers.push_back(
                {
                    .Buffer         = StagingBufferResource.Buffer.get(),
                    .Image          = TextureArrayResource2.Image.get(),
                    .Width          = TextureArrayResource2.Width,
                    .Height         = TextureArrayResource2.Height,
                }
            );
            
            BufferToImageTransferRecorder.ImageTransfers[1].AddTransfer(CombinedImageSize,0,TextureArrayResource2.ImageCount);
            
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
                2
            );
            
            DescriptorPoolSizes.SetDescriptorPoolSize(
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                2
            );
            
            DescriptorPoolCreateInfo.maxSets = 4;
            
            DescriptorPoolSizes.AssignToDescriptorPoolCreateInfo(DescriptorPoolCreateInfo);
            
            std::cout << "Creating descriptor pool" << std::endl;
            VkDescriptorPool NewDescriptorPool = VK_NULL_HANDLE;
            if(vkCreateDescriptorPool(DemoFramework.Device.get(),&DescriptorPoolCreateInfo,nullptr,&NewDescriptorPool) != VK_SUCCESS)
            {
                std::cout << "Error while creating descriptor pool" << std::endl;
            }
            DescriptorPool = lvkDescriptorPool{NewDescriptorPool,DemoFramework.Device.get()};
            
            lvkDescriptorPoolUtil DescriptorPoolUtil(DemoFramework.Device.get(),DescriptorPool.get());
            
            VkDescriptorSet UboDescriptorSet1 = DescriptorPoolUtil.Allocate(UniformBufferLayout.get());
            VkDescriptorSet UboDescriptorSet2 = DescriptorPoolUtil.Allocate(UniformBufferLayout.get());
            VkDescriptorSet TexDescriptorSet1 = DescriptorPoolUtil.Allocate(TextureArrayLayout.get());
            VkDescriptorSet TexDescriptorSet2 = DescriptorPoolUtil.Allocate(TextureArrayLayout.get());
            
            {
                VkDescriptorBufferInfo BufferInfo1 = {};
                BufferInfo1.buffer = UniformBufferResource1.Buffer.get();
                BufferInfo1.offset = 0;
                BufferInfo1.range  = StructSize;
                
                VkWriteDescriptorSet UboDescriptorWrite1 = {};
                UboDescriptorWrite1.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                UboDescriptorWrite1.dstSet          = UboDescriptorSet1;
                UboDescriptorWrite1.dstBinding      = 0;
                UboDescriptorWrite1.dstArrayElement = 0;
                
                UboDescriptorWrite1.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                UboDescriptorWrite1.descriptorCount = 1;
                
                UboDescriptorWrite1.pBufferInfo = &BufferInfo1;
                
                vkUpdateDescriptorSets(DemoFramework.Device.get(),1,&UboDescriptorWrite1,0,nullptr);
                VkDescriptorBufferInfo BufferInfo2 = {};
                BufferInfo2.buffer = UniformBufferResource2.Buffer.get();
                BufferInfo2.offset = 0;
                BufferInfo2.range  = StructSize;
                
                VkWriteDescriptorSet UboDescriptorWrite2 = {};
                UboDescriptorWrite2.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                UboDescriptorWrite2.dstSet          = UboDescriptorSet2;
                UboDescriptorWrite2.dstBinding      = 0;
                UboDescriptorWrite2.dstArrayElement = 0;
                
                UboDescriptorWrite2.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                UboDescriptorWrite2.descriptorCount = 1;
                
                UboDescriptorWrite2.pBufferInfo = &BufferInfo2;
                
                vkUpdateDescriptorSets(DemoFramework.Device.get(),1,&UboDescriptorWrite2,0,nullptr);
                
                VkDescriptorImageInfo ImageInfo1 = {};
                ImageInfo1.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                ImageInfo1.imageView   = TextureImgView1.get();
                
                VkWriteDescriptorSet TexDescriptorWrite1 = {};
                
                TexDescriptorWrite1.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                TexDescriptorWrite1.dstSet          = TexDescriptorSet1;
                TexDescriptorWrite1.dstBinding      = 1;
                TexDescriptorWrite1.dstArrayElement = 0;
                
                TexDescriptorWrite1.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                TexDescriptorWrite1.descriptorCount = 1;
                
                TexDescriptorWrite1.pImageInfo = &ImageInfo1;
                
                vkUpdateDescriptorSets(DemoFramework.Device.get(),1,&TexDescriptorWrite1,0,nullptr);
                
                VkDescriptorImageInfo ImageInfo2 = {};
                ImageInfo2.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                ImageInfo2.imageView   = TextureImgView2.get();
                
                VkWriteDescriptorSet TexDescriptorWrite2 = {};
                
                TexDescriptorWrite2.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                TexDescriptorWrite2.dstSet          = TexDescriptorSet2;
                TexDescriptorWrite2.dstBinding      = 1;
                TexDescriptorWrite2.dstArrayElement = 0;
                
                TexDescriptorWrite2.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                TexDescriptorWrite2.descriptorCount = 1;
                
                TexDescriptorWrite2.pImageInfo = &ImageInfo2;
                
                vkUpdateDescriptorSets(DemoFramework.Device.get(),1,&TexDescriptorWrite2,0,nullptr);
            }
            
            TexArrayDescriptorSet1 = {
                .PipelineLayout = PipelineLayout.get(),
                .DescriptorSet  = TexDescriptorSet1
            };
            
            TexArrayDescriptorSet2 = {
                .PipelineLayout = PipelineLayout.get(),
                .DescriptorSet  = TexDescriptorSet2
            };
            
            UniformBufferDescriptorSet1 = {
                .PipelineLayout = PipelineLayout.get(),
                .DescriptorSet  = UboDescriptorSet1,
                .Offset         = 0,
                .StructSize     = StructSize
            };
            
            UniformBufferDescriptorSet2 = {
                .PipelineLayout = PipelineLayout.get(),
                .DescriptorSet  = UboDescriptorSet2,
                .Offset         = 0,
                .StructSize     = StructSize
            };
        }
        
        {
            constexpr unsigned int NUM_VERTICES = 7;
            constexpr unsigned int NUM_COMPONENTS = 2;
            float VertexData[NUM_VERTICES*NUM_COMPONENTS] = {-0.125,-0.125,-0.125,0.125,0.125,0.125,0.125,-0.125,0.2,0.0,0.4,0.0,0.4,0.2};
            float TexCoordData[NUM_VERTICES*NUM_COMPONENTS] = {0.0,0.0,0.0,1.0,1.0,1.0,1.0,0.0,0.0,0.0,0.0,1.0,1.0,1.0};
            
            constexpr unsigned int NUM_INDICES = 6;
            std::uint32_t IndexData[NUM_INDICES] = {0,1,2,0,3,2};
            
            VertexBufferResource = BufferFactory.CreateBuffer(sizeof(VertexData) + sizeof(TexCoordData),VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
            {
                lvkMappedMemory MappedMemory = VertexBufferResource.Allocation.Map();
                lMemoryView FullMemoryView = MappedMemory.GetView();
                
                lMemoryView VertexMemoryView = FullMemoryView.GetView(0,sizeof(VertexData));
                VertexMemoryView.MemCpySec(VertexData,sizeof(VertexData));
                lMemoryView TexCoordMemoryView = FullMemoryView.GetView(sizeof(VertexData),sizeof(TexCoordData));
                TexCoordMemoryView.MemCpySec(TexCoordData,sizeof(TexCoordData));
            }
            
            IndexBufferResource = BufferFactory.CreateBuffer(sizeof(IndexData),VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
            {
                lvkMappedMemory MappedMemory = IndexBufferResource.Allocation.Map();
                lMemoryView FullMemoryView = MappedMemory.GetView();
                
                FullMemoryView.MemCpySec(IndexData,sizeof(IndexData));
            }
            
            MeshVertexBuffers = lvkMeshGroupVertexData{2,{{VertexBufferResource.Buffer.get(),VertexBufferResource.Buffer.get()}},{{0,sizeof(VertexData)}}};
            MeshIndexBuffer   = lvkMeshGroupIndexData{IndexBufferResource.Buffer.get(),0};
            
            MeshGroup.SubMeshes.push_back(
                {
                    .BaseVertex  = 4,
                    .VertexCount = 3
                }
            );
            MeshGroup.IndexedSubMeshes.push_back(
                {
                    .BaseVertex = 0,
                    .FirstIndex = 0,
                    .IndexCount = 6,
                }
            );
        }
        
        {
            PipelineNode.Pipeline = Pipeline.get();
            PipelineNode.Children.resize(2);
            PipelineNode.Children[0] = &TextureGroup1;
            PipelineNode.Children[1] = &TextureGroup2;
            
            TextureGroup1.UniformBindings = &TexArrayDescriptorSet1;
            TextureGroup1.Children.resize(1);
            TextureGroup1.Children[0] = &MeshGroup1;
            
            TextureGroup2.UniformBindings = &TexArrayDescriptorSet2;
            TextureGroup2.Children.resize(1);
            TextureGroup2.Children[0] = &MeshGroup2;
            
            MeshGroup1.VertexData = &MeshVertexBuffers;
            MeshGroup1.IndexData  = &MeshIndexBuffer;
            MeshGroup1.MeshGroup  = &MeshGroup;
            MeshGroup1.Children.resize(1);
            MeshGroup1.Children[0] = &UniformSubBuffers1;
            
            MeshGroup2.VertexData = &MeshVertexBuffers;
            MeshGroup2.IndexData  = &MeshIndexBuffer;
            MeshGroup2.MeshGroup  = &MeshGroup;
            MeshGroup2.Children.resize(1);
            MeshGroup2.Children[0] = &UniformSubBuffers2;
            
            UniformSubBuffers1.DynamicUniformBindings = &UniformBufferDescriptorSet1;
            UniformSubBuffers1.Children.resize(NumTriangles);
            for(int i=0;i < NumTriangles;i++)
                {UniformSubBuffers1.Children[i] = &TwoMeshes;}
            
            UniformSubBuffers2.DynamicUniformBindings = &UniformBufferDescriptorSet2;
            UniformSubBuffers2.Children.resize(NumTriangles);
            for(int i=0;i < NumTriangles;i++)
                {UniformSubBuffers2.Children[i] = &TwoMeshes;}
            
            TwoMeshes.Children.resize(2);
            TwoMeshes.Children[0] = &Mesh1;
            TwoMeshes.Children[1] = &Mesh2;
            
            Mesh1.Indexed = true;
            Mesh1.MeshId = 0;
            
            Mesh2.Indexed = false;
            Mesh2.MeshId = 0;
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
                        
                        lvkHierarchyRecorder CommandRecorder(CommandBuffers[i]);
                        PipelineNode.Accept(CommandRecorder);
                    },
                    *Swapchain.get()
                )
            );
        }
        
        TaskGraph.Initialize(DemoFramework.Device.get(),DemoFramework.GraphicsQueue,DemoFramework.PresentQueue);
    }
    
    ~lvkResourceHierarchyApp()
    {
        vkDeviceWaitIdle(DemoFramework.Device.get());
    }
};

const bool IsResizable = true;

std::unique_ptr<IVKSampleApp> CreateSampleApp(IVKWindow &window)
{
    return std::unique_ptr<IVKSampleApp>(new lvkResourceHierarchyApp(window));
}
