#ifndef LVK_RESOURCE_TEST_RENDERER_H
#define LVK_RESOURCE_TEST_RENDERER_H

#include <lvkResourceManager/lvkResourceGroupLoader.h>

struct lvkTestRendererDescriptors
{
    lvkDescriptorPool            DescriptorPool;
    
    VkDescriptorSet              UboDescriptorSet;
    std::vector<VkDescriptorSet> TextureDescriptorSets;
    
    void Initialize(VkDevice device,VkDescriptorSetLayout ubo_layout,VkDescriptorSetLayout texture_layout,std::size_t texture_array_count)
    {
        lvkDescriptorPoolSizes     DescriptorPoolSizes;
        VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo = LVK_EMPTY_DESCRIPTOR_POOL_CREATE_INFO;
        
        DescriptorPoolSizes.SetDescriptorPoolSize(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            1
        );
        
        DescriptorPoolSizes.SetDescriptorPoolSize(
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            texture_array_count
        );
        
        DescriptorPoolCreateInfo.maxSets = 1 + texture_array_count;
        
        DescriptorPoolSizes.AssignToDescriptorPoolCreateInfo(DescriptorPoolCreateInfo);
        
        std::cout << "Creating descriptor pool" << std::endl;
        VkDescriptorPool NewDescriptorPool = VK_NULL_HANDLE;
        if(vkCreateDescriptorPool(device,&DescriptorPoolCreateInfo,nullptr,&NewDescriptorPool) != VK_SUCCESS)
        {
            std::cout << "Error while creating descriptor pool" << std::endl;
        }
        DescriptorPool = lvkDescriptorPool{NewDescriptorPool,device};
        
        lvkDescriptorPoolUtil DescriptorPoolUtil(device,DescriptorPool.get());
        
        UboDescriptorSet      = DescriptorPoolUtil.Allocate(ubo_layout);
        TextureDescriptorSets = DescriptorPoolUtil.Allocate({texture_array_count,texture_layout});
    }
};

struct lvkTestPipelineLayout
{
    lvkSampler             TextureImgSampler;
    
    lvkDescriptorSetLayout UniformBufferLayout;
    lvkDescriptorSetLayout TextureArrayLayout;
    lvkPipelineLayout      PipelineLayout;
    
    void Initialize(VkDevice device)
    {
        {
            VkSamplerCreateInfo SamplerCreateInfo = LVK_LINEAR_CLAMPED_SAMPLER_CREATE_INFO;
            
            VkSampler Sampler = VK_NULL_HANDLE;
            if(vkCreateSampler(device,&SamplerCreateInfo,nullptr,&Sampler) != VK_SUCCESS)
            {
                std::cout << "Error while creating sampler" << std::endl;
            }
            
            TextureImgSampler = lvkSampler{Sampler,{device}};
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
            if(vkCreateDescriptorSetLayout(device, &DescriptorSetLayoutCreateInfo, nullptr, &TmpDescriptorSetLayout) != VK_SUCCESS)
            {
                std::cout << "Error while creating descriptor set layout" << std::endl;
            }
            
            UniformBufferLayout = lvkDescriptorSetLayout{TmpDescriptorSetLayout,{device}};
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
            if(vkCreateDescriptorSetLayout(device, &DescriptorSetLayoutCreateInfo, nullptr, &TmpDescriptorSetLayout) != VK_SUCCESS)
            {
                std::cout << "Error while creating descriptor set layout" << std::endl;
            }
            
            TextureArrayLayout = lvkDescriptorSetLayout{TmpDescriptorSetLayout,{device}};
        }
        
        {
            VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo = LVK_EMPTY_PIPELINE_LAYOUT_CREATE_INFO;
            
            std::array<VkDescriptorSetLayout,2> DescriptorSetLayouts = {UniformBufferLayout.get(),TextureArrayLayout.get()};
            
            PipelineLayoutCreateInfo.setLayoutCount = DescriptorSetLayouts.size();
            PipelineLayoutCreateInfo.pSetLayouts    = DescriptorSetLayouts.data();
            
            std::cout << "Creating pipeline layout" << std::endl;
            VkPipelineLayout NewPipelineLayout;
            if(vkCreatePipelineLayout(device,&PipelineLayoutCreateInfo,nullptr,&NewPipelineLayout) != VK_SUCCESS)
            {
                std::cout << "Error while creating pipeline layout" << std::endl;
            }
            
            PipelineLayout = lvkPipelineLayout{NewPipelineLayout,{device}};
        }
    }
};

struct lvkImagePoolDescriptorSet
{
    lvkImagePool   *ImagePool;
    lvkImageView    PoolView;
    VkDescriptorSet PoolDescriptorSet;
};

struct lvkTestRenderer
{
    lvkTestPipelineLayout      TestPipelineLayout;
    lvkTestRendererDescriptors Descriptors;
    
    std::vector<lvkImagePoolDescriptorSet> TexturePoolDescriptorSets;
    
    lvkPipeline CreateInterleavedPipeline(
        VkDevice device,
        VkRenderPass render_pass,
        std::uint32_t subpass,
        VkShaderModule vertex_shader,
        VkShaderModule fragment_shader
    )
    {
        lvkDemoShaderStages           DemoShaderStages;
        lvkPipelineFixedFunctionState FixedFunctionState;
        lvkPipelineDynamicState       DynamicState;
        lvkDemoColorBlendAttachments  DemoColorBlendAttachments;
        VkGraphicsPipelineCreateInfo  PipelineCreateInfo = LVK_EMPTY_PIPELINE_CREATE_INFO;
        
        lvkVertexInputDescription VertexInputDescription;
        
        VertexInputDescription.AddVertexInputBinding(0,8*sizeof(float),VK_VERTEX_INPUT_RATE_VERTEX);
        VertexInputDescription.AddVertexInputAttributeDescription(0,0,VK_FORMAT_R32G32B32_SFLOAT,0);
        VertexInputDescription.AddVertexInputAttributeDescription(0,1,VK_FORMAT_R32G32B32_SFLOAT,3*sizeof(float));
        VertexInputDescription.AddVertexInputAttributeDescription(0,2,VK_FORMAT_R32G32_SFLOAT,6*sizeof(float));
        
        DemoShaderStages.SetShaders(vertex_shader,fragment_shader);
        
        DynamicState.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
        DynamicState.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR);
        
        PipelineCreateInfo.renderPass = render_pass;
        PipelineCreateInfo.subpass    = subpass;
        PipelineCreateInfo.layout     = TestPipelineLayout.PipelineLayout.get();
        
        VertexInputDescription.AssignToPipelineCreateInfo(PipelineCreateInfo);
        DemoShaderStages.AssignToPipelineCreateInfo(PipelineCreateInfo);
        FixedFunctionState.AssignToPipelineCreateInfo(PipelineCreateInfo);
        DynamicState.AssignToPipelineCreateInfo(PipelineCreateInfo);
        DemoColorBlendAttachments.AssignToPipelineCreateInfo(PipelineCreateInfo);
        
        VkPipelineDepthStencilStateCreateInfo PipelineDepthStencilInfo = {};
        
        PipelineDepthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        PipelineDepthStencilInfo.depthTestEnable  = VK_TRUE;
        PipelineDepthStencilInfo.depthWriteEnable = VK_TRUE;
        
        PipelineDepthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        
        PipelineDepthStencilInfo.depthBoundsTestEnable = VK_FALSE;
        PipelineDepthStencilInfo.minDepthBounds = 0.0f;
        PipelineDepthStencilInfo.maxDepthBounds = 1.0f;
        
        PipelineDepthStencilInfo.stencilTestEnable = VK_FALSE;
        
        PipelineCreateInfo.pDepthStencilState = &PipelineDepthStencilInfo;
        
        std::cout << "Creating graphics pipeline" << std::endl;
        VkPipeline NewPipeline;
        if(vkCreateGraphicsPipelines(device,VK_NULL_HANDLE,1,&PipelineCreateInfo,nullptr,&NewPipeline) != VK_SUCCESS)
        {
            std::cout << "Error while creating pipeline" << std::endl;
        }
        
        return lvkPipeline{NewPipeline,{device}};
    }
    
    lvkPipeline CreateDeinterleavedPipeline(
        VkDevice device,
        VkRenderPass render_pass,
        std::uint32_t subpass,
        VkShaderModule vertex_shader,
        VkShaderModule fragment_shader
    )
    {
        lvkDemoShaderStages           DemoShaderStages;
        lvkPipelineFixedFunctionState FixedFunctionState;
        lvkPipelineDynamicState       DynamicState;
        lvkDemoColorBlendAttachments  DemoColorBlendAttachments;
        VkGraphicsPipelineCreateInfo  PipelineCreateInfo = LVK_EMPTY_PIPELINE_CREATE_INFO;
        
        lvkVertexInputDescription VertexInputDescription;
        
        VertexInputDescription.AddVertexInputBinding(0,3*sizeof(float),VK_VERTEX_INPUT_RATE_VERTEX);
        VertexInputDescription.AddVertexInputBinding(1,3*sizeof(float),VK_VERTEX_INPUT_RATE_VERTEX);
        VertexInputDescription.AddVertexInputBinding(2,2*sizeof(float),VK_VERTEX_INPUT_RATE_VERTEX);
        VertexInputDescription.AddVertexInputAttributeDescription(0,0,VK_FORMAT_R32G32B32_SFLOAT,0);
        VertexInputDescription.AddVertexInputAttributeDescription(1,1,VK_FORMAT_R32G32B32_SFLOAT,0);
        VertexInputDescription.AddVertexInputAttributeDescription(2,2,VK_FORMAT_R32G32_SFLOAT,0);
        
        DemoShaderStages.SetShaders(vertex_shader,fragment_shader);
        
        DynamicState.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
        DynamicState.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR);
        
        PipelineCreateInfo.renderPass = render_pass;
        PipelineCreateInfo.subpass    = subpass;
        PipelineCreateInfo.layout     = TestPipelineLayout.PipelineLayout.get();
        
        VertexInputDescription.AssignToPipelineCreateInfo(PipelineCreateInfo);
        DemoShaderStages.AssignToPipelineCreateInfo(PipelineCreateInfo);
        FixedFunctionState.AssignToPipelineCreateInfo(PipelineCreateInfo);
        DynamicState.AssignToPipelineCreateInfo(PipelineCreateInfo);
        DemoColorBlendAttachments.AssignToPipelineCreateInfo(PipelineCreateInfo);
        
        VkPipelineDepthStencilStateCreateInfo PipelineDepthStencilInfo = {};
        
        PipelineDepthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        PipelineDepthStencilInfo.depthTestEnable  = VK_TRUE;
        PipelineDepthStencilInfo.depthWriteEnable = VK_TRUE;
        
        PipelineDepthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        
        PipelineDepthStencilInfo.depthBoundsTestEnable = VK_FALSE;
        PipelineDepthStencilInfo.minDepthBounds = 0.0f;
        PipelineDepthStencilInfo.maxDepthBounds = 1.0f;
        
        PipelineDepthStencilInfo.stencilTestEnable = VK_FALSE;
        
        PipelineCreateInfo.pDepthStencilState = &PipelineDepthStencilInfo;
        
        std::cout << "Creating graphics pipeline" << std::endl;
        VkPipeline NewPipeline;
        if(vkCreateGraphicsPipelines(device,VK_NULL_HANDLE,1,&PipelineCreateInfo,nullptr,&NewPipeline) != VK_SUCCESS)
        {
            std::cout << "Error while creating pipeline" << std::endl;
        }
        
        return lvkPipeline{NewPipeline,{device}};
    }
    
    void WritePoolDescriptorSets(VkDevice device,lvkImagePoolCategory &image_pool_category)
    {
        if(image_pool_category.Instances.size() > 0)
        {
            lvkTextureDescriptorWriteBatch TextureWriteBatch(
                device,
                image_pool_category.Instances.size()
            );
            
            std::size_t CurrentId = 0;
            for(auto &TexturePool : image_pool_category.Instances)
            {
                TexturePoolDescriptorSets.push_back({
                    .ImagePool = &TexturePool,
                    .PoolView = TexturePool.ImagePoolResource.CreateImageView(device,image_pool_category.TextureFormat),
                    .PoolDescriptorSet = Descriptors.TextureDescriptorSets[CurrentId]
                });
                
                auto &CurrentPool = TexturePoolDescriptorSets.back();
                
                TextureWriteBatch.SetDescriptorWrite(CurrentId,CurrentPool.PoolView.get(),CurrentPool.PoolDescriptorSet);
                
                CurrentId++;
            }
        }
    }
    
    void BindMeshGroup(VkCommandBuffer command_buffer,lvkMeshGroupData &mesh_group_data)
    {
        constexpr std::uint32_t               BindingCount = 16;
        std::array<VkBuffer,BindingCount>     VertexBuffers;
        std::array<VkDeviceSize,BindingCount> VertexBufferOffsets;
        
        for(std::size_t i=0;i < mesh_group_data.BindingCount;i++)
        {
            VertexBuffers[i]       = mesh_group_data.VertexBuffer;
            VertexBufferOffsets[i] = mesh_group_data.BindingsOffsets[i];
        }
        
        vkCmdBindVertexBuffers(command_buffer,0,mesh_group_data.BindingCount,VertexBuffers.data(),VertexBufferOffsets.data());
        
        VkBuffer     IndexBuffer       = mesh_group_data.IndexBuffer;
        VkDeviceSize IndexBufferOffset = mesh_group_data.IndexDataRegion.GetOffset();
        
        vkCmdBindIndexBuffer(command_buffer,IndexBuffer,IndexBufferOffset,VK_INDEX_TYPE_UINT32);
    }
    
    VkDescriptorSet GetImageGroupDescriptor(lvkImageGroupData &image_group_data)
    {
        for(std::size_t j=0;j < TexturePoolDescriptorSets.size();j++)
        {
            if(TexturePoolDescriptorSets[j].ImagePool == image_group_data.Image)
            {
                return TexturePoolDescriptorSets[j].PoolDescriptorSet;
            }
        }
        
        return VK_NULL_HANDLE;
    }
    
    void BindImageGroup(VkCommandBuffer command_buffer,lvkImageGroupData &image_group_data)
    {
        /*
        for(std::size_t j=0;j < TexturePoolDescriptorSets.size();j++)
        {
            if(TexturePoolDescriptorSets[j].ImagePool == image_group_data.Image)
            {
                vkCmdBindDescriptorSets(
                    command_buffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    TestPipelineLayout.PipelineLayout.get(),
                    1,
                    1,
                    &TexturePoolDescriptorSets[j].PoolDescriptorSet,
                    0,
                    nullptr
                );
                break;
            }
        }
        */
        VkDescriptorSet PoolDescriptorSet = GetImageGroupDescriptor(image_group_data);
        vkCmdBindDescriptorSets(
            command_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            TestPipelineLayout.PipelineLayout.get(),
            1,
            1,
            &PoolDescriptorSet,
            0,
            nullptr
        );
    }
    
    static void DrawIndexedMesh(VkCommandBuffer command_buffer,lvkMeshGroupData &mesh_group_data,std::size_t mesh_id,std::size_t submesh_id)
    {
        std::size_t IndexedSubMeshId = mesh_group_data.MeshGroup.Meshes[mesh_id].FirstIndexedSubMesh + submesh_id;
        
        vkCmdDrawIndexed(
            command_buffer,
            mesh_group_data.MeshGroup.IndexedSubMeshes[IndexedSubMeshId].IndexCount,
            1,
            mesh_group_data.MeshGroup.IndexedSubMeshes[IndexedSubMeshId].FirstIndex,
            mesh_group_data.MeshGroup.IndexedSubMeshes[IndexedSubMeshId].BaseVertex,
            0
        );
    }
};

struct lvkTestPools
{
    lvkMemoryUtility MemoryUtility;
    
    std::unique_ptr<lvkBufferPoolCategory> VertexBufferCategory;
    std::unique_ptr<lvkBufferPoolCategory> IndexBufferCategory;
    
    std::unique_ptr<lvkImagePoolCategory> TextureCategory;
    
    std::unique_ptr<lvkPMappedBufferPoolCategory> StagingBufferCategory;
    
    struct lvkTestPoolParams
    {
        std::size_t MeshBufferPoolSize;
        std::size_t StagingBufferPoolSize;
        std::size_t TextureWidth;
        std::size_t TextureHeight;
        std::size_t TextureCount;
        VkFormat    TextureFormat;
    };
    
    void Initialize(VkDevice device,VkPhysicalDevice physical_device,const lvkTestPoolParams &pool_params)
    {
        MemoryUtility = {device,physical_device};
        
        {
            std::unique_ptr<lvkMemoryAllocProcElement> DevLocalAllocator = MemoryUtility.CreateAllocator(LVK_STATIC_MEMORY_PROPERTIES);
            std::unique_ptr<lvkMemoryAllocProcElement> FallbackAllocator = MemoryUtility.CreateAllocator(LVK_FALLBACK_MEMORY_PROPERTIES);
            DevLocalAllocator->SetSuccessor(std::move(FallbackAllocator));
            
            VertexBufferCategory = std::unique_ptr<lvkBufferPoolCategory>(
                new lvkBufferPoolCategory(
                    device,
                    std::move(DevLocalAllocator),
                    pool_params.MeshBufferPoolSize,
                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
                )
            );
        }
        
        {
            std::unique_ptr<lvkMemoryAllocProcElement> DevLocalAllocator = MemoryUtility.CreateAllocator(LVK_STATIC_MEMORY_PROPERTIES);
            std::unique_ptr<lvkMemoryAllocProcElement> FallbackAllocator = MemoryUtility.CreateAllocator(LVK_FALLBACK_MEMORY_PROPERTIES);
            DevLocalAllocator->SetSuccessor(std::move(FallbackAllocator));
            
            IndexBufferCategory = std::unique_ptr<lvkBufferPoolCategory>(
                new lvkBufferPoolCategory(
                    device,
                    std::move(DevLocalAllocator),
                    pool_params.MeshBufferPoolSize,
                    VK_BUFFER_USAGE_INDEX_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_DST_BIT
                )
            );
        }
        
        {
            std::unique_ptr<lvkMemoryAllocProcElement> TransferAllocator = MemoryUtility.CreateAllocator(LVK_TRANSFER_MEMORY_PROPERTIES);
            
            StagingBufferCategory = std::unique_ptr<lvkPMappedBufferPoolCategory>(
                new lvkPMappedBufferPoolCategory(
                    device,
                    std::move(TransferAllocator),
                    pool_params.StagingBufferPoolSize,
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT
                )
            );
        }
        
        {
            std::unique_ptr<lvkMemoryAllocProcElement> DevLocalAllocator = MemoryUtility.CreateAllocator(LVK_STATIC_MEMORY_PROPERTIES);
            std::unique_ptr<lvkMemoryAllocProcElement> FallbackAllocator = MemoryUtility.CreateAllocator(LVK_FALLBACK_MEMORY_PROPERTIES);
            DevLocalAllocator->SetSuccessor(std::move(FallbackAllocator));
            
            TextureCategory = std::unique_ptr<lvkImagePoolCategory>(
                new lvkImagePoolCategory(
                    device,
                    std::move(DevLocalAllocator),
                    pool_params.TextureWidth,
                    pool_params.TextureHeight,
                    pool_params.TextureFormat,
                    pool_params.TextureCount,
                    VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
                )
            );
        }
    }
};

struct lvkTestUniformBuffer
{
    lvkBufferResource UniformBufferResource;
    lvkMappedMemory   MappedMemory;
    std::uint32_t     StructSize;
    VkDescriptorSet   DescriptorSet;
    
    void BindBlock(VkCommandBuffer command_buffer,VkPipelineLayout pipeline_layout,std::size_t block_id)
    {
        std::uint32_t DynamicOffset = block_id*StructSize;
        
        vkCmdBindDescriptorSets(
            command_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeline_layout,
            0,
            1,
            &DescriptorSet,
            1,
            &DynamicOffset
        );
    }
    
    void WriteDescriptorSet(VkDevice device,VkDescriptorSet descriptor_set)
    {
        VkDescriptorBufferInfo BufferInfo = {};
        BufferInfo.buffer = UniformBufferResource.Buffer.get();
        BufferInfo.offset = 0;
        BufferInfo.range  = StructSize;
        
        VkWriteDescriptorSet UboDescriptorWrite = {};
        UboDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        UboDescriptorWrite.dstSet          = descriptor_set;
        UboDescriptorWrite.dstBinding      = 0;
        UboDescriptorWrite.dstArrayElement = 0;
        
        UboDescriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        UboDescriptorWrite.descriptorCount = 1;
        
        UboDescriptorWrite.pBufferInfo = &BufferInfo;
        
        vkUpdateDescriptorSets(device,1,&UboDescriptorWrite,0,nullptr);
        
        DescriptorSet = descriptor_set;
    }
    
    void Initialize(VkDevice device,VkPhysicalDevice physical_device,std::size_t sizeof_struct,std::size_t struct_count)
    {
        lvkMemoryUtility MemoryUtility(device,physical_device);
        std::unique_ptr<lvkMemoryAllocProcElement> Allocator = MemoryUtility.CreateAllocator(LVK_TRANSFER_MEMORY_PROPERTIES);
        
        lvkResourceFactory BufferFactory(
            device,
            std::move(Allocator)
        );
        
        VkPhysicalDeviceProperties DeviceProperties;
        vkGetPhysicalDeviceProperties(physical_device,&DeviceProperties);
        
        StructSize = lPaddedStructSize(DeviceProperties.limits.minUniformBufferOffsetAlignment,sizeof_struct);
        
        std::uint32_t UniformBufferSize = struct_count * StructSize;
        UniformBufferResource = BufferFactory.CreateBuffer(UniformBufferSize,VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        
        MappedMemory = UniformBufferResource.Allocation.Map();
    }
};

#endif // LVK_RESOURCE_TEST_RENDERER_H
