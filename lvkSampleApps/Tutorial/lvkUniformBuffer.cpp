
#include <lvkSampleApps/lvkDemoUtils.h>

#include <lvkUtils/lvkShaderModuleLibrary.h>

#include <lvkUtils/lvkSmartPtrs/lvkPipelineLayout.h>
#include <lvkUtils/lvkSmartPtrs/lvkDescriptorSetLayout.h>

#include <lvkUtils/lvkDescriptorPoolUtils.h>

class lvkUniformBufferApp : public IVKSampleApp
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
    
    lvkDescriptorSetLayout DescriptorSetLayout;
    lvkPipelineLayout      PipelineLayout;
    
    lvkPipeline Pipeline;
    
    lvkBufferResource UniformBufferResource;
    
    lvkDescriptorPool DescriptorPool;
    
    std::vector<VkDescriptorSet> DescriptorSets;
    
    std::vector<VkCommandBuffer> CommandBuffers;
    std::unique_ptr<lvkDemoCommandPool> CommandPoolResetObserver;
    std::vector<std::unique_ptr<lvkDemoBufferRecorder> > CommandRecorders;
    
    lvkDemoTaskGraph TaskGraph;
    
public:
    
    virtual void Loop() override
    {
        TaskGraph.Submit(DemoFramework.Device.get(),*Swapchain.get(),CommandBuffers.data());
    }
    
    lvkUniformBufferApp(IVKWindow &window)
        :DemoFramework(window)
    {
        ShaderModuleLibrary = {DemoFramework.Device.get()};
        
        Swapchain = std::unique_ptr<lvkSwapchain>(new lvkSwapchain(DemoFramework.Device.get(),DemoFramework.PhysicalDevice,DemoFramework.Surface.get()));
        RenderPass = DemoFramework.CreateSimpleRenderPass(DemoFramework.Device.get(),Swapchain->GetFormat());
        DemoSwapchainFramebuffers = std::unique_ptr<lvkDemoSwapchainFramebuffers>(new lvkDemoSwapchainFramebuffers(DemoFramework.Device.get(),RenderPass.get(),*Swapchain.get()));
        
        {
            {
                VkDescriptorSetLayoutBinding UboLayoutBinding = {};
                UboLayoutBinding.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                UboLayoutBinding.descriptorCount = 1;
                UboLayoutBinding.binding = 0;
                
                UboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
                
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
                
                DescriptorSetLayout = lvkDescriptorSetLayout{TmpDescriptorSetLayout,{DemoFramework.Device.get()}};
            }
            
            VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo = LVK_EMPTY_PIPELINE_LAYOUT_CREATE_INFO;
            
            std::array<VkDescriptorSetLayout,1> DescriptorSetLayouts = {DescriptorSetLayout.get()};
            
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
                ShaderModuleLibrary.GetShaderModule("../Shaders/UboHardcodedVertexShader.spv"),
                ShaderModuleLibrary.GetShaderModule("../Shaders/FragmentShader.spv")
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
        
        VkPhysicalDeviceMemoryProperties MemoryProperties;
        vkGetPhysicalDeviceMemoryProperties(DemoFramework.PhysicalDevice,&MemoryProperties);
        
        std::unique_ptr<lvkMemoryAllocProcElement> Allocator = std::unique_ptr<lvkMemoryAllocProcElement>(
            new lvkMemoryAllocProcElement(DemoFramework.Device.get(),MemoryProperties,LVK_TRANSFER_MEMORY_PROPERTIES)
        );
        
        lvkResourceFactory BufferFactory(
            DemoFramework.Device.get(),
            std::move(Allocator)
        );
        
        VkPhysicalDeviceProperties DeviceProperties;
        vkGetPhysicalDeviceProperties(DemoFramework.PhysicalDevice,&DeviceProperties);
        
        StructSize = lPaddedStructSize(DeviceProperties.limits.minUniformBufferOffsetAlignment,sizeof(TriData));
        
        std::uint32_t UniformBufferSize = NumTriangles * StructSize;
        
        UniformBufferResource = BufferFactory.CreateBuffer(UniformBufferSize,VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        
        {
            lvkMappedMemory MappedMemory = UniformBufferResource.Allocation.Map();
            lMemoryView FullMemoryView = MappedMemory.GetView();
            
            lBlockMemoryView TriDataBufferView((char*)FullMemoryView.GetPtr(),NumTriangles,StructSize);
            
            for(int i=0;i < NumTriangles;i++)
            {
                TriData *TriDataElement = (TriData *)TriDataBufferView.GetBlock(i);
                TriDataElement->x =  -0.8 + i*0.4;
                TriDataElement->y =   0.8 - i*0.4;
            }
        }
        
        {
            lvkDescriptorPoolSizes     DescriptorPoolSizes;
            VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo = LVK_EMPTY_DESCRIPTOR_POOL_CREATE_INFO;
            
            DescriptorPoolSizes.SetDescriptorPoolSize(
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                Swapchain->GetImageCount()
            );
            
            DescriptorPoolCreateInfo.maxSets = Swapchain->GetImageCount();
            
            DescriptorPoolSizes.AssignToDescriptorPoolCreateInfo(DescriptorPoolCreateInfo);
            
            std::cout << "Creating descriptor pool" << std::endl;
            VkDescriptorPool NewDescriptorPool = VK_NULL_HANDLE;
            if(vkCreateDescriptorPool(DemoFramework.Device.get(),&DescriptorPoolCreateInfo,nullptr,&NewDescriptorPool) != VK_SUCCESS)
            {
                std::cout << "Error while creating descriptor pool" << std::endl;
            }
            DescriptorPool = lvkDescriptorPool{NewDescriptorPool,DemoFramework.Device.get()};
            
            lvkDescriptorPoolUtil DescriptorPoolUtil(DemoFramework.Device.get(),DescriptorPool.get());
            
            DescriptorSets = DescriptorPoolUtil.Allocate({Swapchain->GetImageCount(),DescriptorSetLayout.get()});
            
            for(int i=0;i < Swapchain->GetImageCount();i++)
            {
                VkDescriptorBufferInfo BufferInfo = {};
                BufferInfo.buffer = UniformBufferResource.Buffer.get();
                BufferInfo.offset = 0;
                BufferInfo.range  = StructSize;
                
                VkWriteDescriptorSet DescriptorWrite = {};
                DescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                DescriptorWrite.dstSet          = DescriptorSets[i];
                DescriptorWrite.dstBinding      = 0;
                DescriptorWrite.dstArrayElement = 0;
                
                DescriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                DescriptorWrite.descriptorCount = 1;
                
                DescriptorWrite.pBufferInfo = &BufferInfo;
                
                vkUpdateDescriptorSets(DemoFramework.Device.get(),1,&DescriptorWrite,0,nullptr);
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
                        
                        for(int j=0;j < NumTriangles;j++)
                        {
                            uint32_t BufferOffset = j*StructSize;
                            
                            vkCmdBindDescriptorSets(CommandBuffers[i],VK_PIPELINE_BIND_POINT_GRAPHICS,PipelineLayout.get(),0,1,&DescriptorSets[i],1,&BufferOffset);
                            
                            vkCmdDraw(CommandBuffers[i],6,1,0,0);
                        }
                    },
                    *Swapchain.get()
                )
            );
        }
        
        TaskGraph.Initialize(DemoFramework.Device.get(),DemoFramework.GraphicsQueue,DemoFramework.PresentQueue);
    }
    
    ~lvkUniformBufferApp()
    {
        vkDeviceWaitIdle(DemoFramework.Device.get());
    }
};

const bool IsResizable = true;

std::unique_ptr<IVKSampleApp> CreateSampleApp(IVKWindow &window)
{
    return std::unique_ptr<IVKSampleApp>(new lvkUniformBufferApp(window));
}
