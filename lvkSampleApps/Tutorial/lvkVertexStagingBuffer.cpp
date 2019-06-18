
#include <lvkSampleApps/lvkDemoUtils.h>

#include <lvkUtils/lvkShaderModuleLibrary.h>

class lvkVertexStagingBufferApp : public IVKSampleApp
{
private:
    lvkDemoFramework DemoFramework;
    
    lvkShaderModuleLibrary ShaderModuleLibrary;
    
    std::unique_ptr<lvkSwapchain> Swapchain;
    lvkRenderPass RenderPass;
    std::unique_ptr<lvkDemoSwapchainFramebuffers> DemoSwapchainFramebuffers;
    
    lvkPipelineLayout PipelineLayout;
    lvkPipeline Pipeline;
    
    lvkBufferResource VertexBufferResource;
    
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
            StagingBufferResource = {};
            TransferCommandPool   = nullptr;
        }
        
        TaskGraph.Submit(DemoFramework.Device.get(),*Swapchain.get(),CommandBuffers.data());
    }
    
    lvkVertexStagingBufferApp(IVKWindow &window)
        :DemoFramework(window)
    {
        ShaderModuleLibrary = {DemoFramework.Device.get()};
        
        Swapchain = std::unique_ptr<lvkSwapchain>(new lvkSwapchain(DemoFramework.Device.get(),DemoFramework.PhysicalDevice,DemoFramework.Surface.get()));
        RenderPass = DemoFramework.CreateSimpleRenderPass(DemoFramework.Device.get(),Swapchain->GetFormat());
        DemoSwapchainFramebuffers = std::unique_ptr<lvkDemoSwapchainFramebuffers>(new lvkDemoSwapchainFramebuffers(DemoFramework.Device.get(),RenderPass.get(),*Swapchain.get()));
        
        PipelineLayout = DemoFramework.CreateEmptyPipelineLayout();
        
        {
            lvkDemoShaderStages           DemoShaderStages;
            lvkPipelineFixedFunctionState FixedFunctionState;
            lvkPipelineDynamicState       DynamicState;
            lvkDemoColorBlendAttachments  DemoColorBlendAttachments;
            VkGraphicsPipelineCreateInfo  PipelineCreateInfo = LVK_EMPTY_PIPELINE_CREATE_INFO;
            
            lvkVertexInputDescription VertexInputDescription;
            
            VertexInputDescription.AddVertexInputBinding(0,2*sizeof(float),VK_VERTEX_INPUT_RATE_VERTEX);
            VertexInputDescription.AddVertexInputAttributeDescription(0,0,VK_FORMAT_R32G32_SFLOAT,0);
            
            DemoShaderStages.SetShaders(
                ShaderModuleLibrary.GetShaderModule("../Shaders/VboVertexShader.spv"),
                ShaderModuleLibrary.GetShaderModule("../Shaders/FragmentShader.spv")
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
        
        constexpr unsigned int NUM_COMPONENTS = 2;
        constexpr unsigned int NUM_VERTICES = 3;
        
        float VertexData[NUM_VERTICES*NUM_COMPONENTS] = {-0.5,1.0,-0.5,0.0,0.5,0.0};
            
        VkPhysicalDeviceMemoryProperties MemoryProperties;
        vkGetPhysicalDeviceMemoryProperties(DemoFramework.PhysicalDevice,&MemoryProperties);
        
        std::unique_ptr<lvkMemoryAllocProcElement> StagingAllocator = std::unique_ptr<lvkMemoryAllocProcElement>(
            new lvkMemoryAllocProcElement(DemoFramework.Device.get(),MemoryProperties,LVK_TRANSFER_MEMORY_PROPERTIES)
        );
        
        lvkResourceFactory StagingBufferFactory(
            DemoFramework.Device.get(),
            std::move(StagingAllocator)
        );
        
        StagingBufferResource = StagingBufferFactory.CreateBuffer(sizeof(VertexData),VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        {
            lvkMappedMemory MappedMemory = StagingBufferResource.Allocation.Map();
            lMemoryView FullMemoryView = MappedMemory.GetView();
            
            FullMemoryView.MemCpySec(VertexData,sizeof(VertexData));
        }
        
        lvkMemoryUtility MemoryUtility(DemoFramework.Device.get(),DemoFramework.PhysicalDevice);
        
        std::unique_ptr<lvkMemoryAllocProcElement> DevLocalAllocator = MemoryUtility.CreateAllocator(LVK_STATIC_MEMORY_PROPERTIES);
        DevLocalAllocator->SetSuccessor(
            MemoryUtility.CreateAllocator(LVK_TRANSFER_MEMORY_PROPERTIES)
        );
        
        lvkResourceFactory DevLocalBufferFactory(
            DemoFramework.Device.get(),
            std::move(DevLocalAllocator)
        );
        
        VertexBufferResource  = DevLocalBufferFactory.CreateBuffer(sizeof(VertexData),VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        
        if(VertexBufferResource.Allocation.IsResident())
            {std::cout << "Successfully allocated resident vertex buffer." << std::endl;}
        if(VertexBufferResource.Allocation.IsHostVisible())
            {std::cout << "Vertex buffer is not host visible." << std::endl;}
        
        TransferTaskGraph.Initialize(DemoFramework.Device.get(),DemoFramework.GraphicsQueue);
        {
            lvkBufferToBufferTransferRecorder BufferToBufferTransferRecorder;
            
            BufferToBufferTransferRecorder.BufferTransfers.push_back({
                .SrcBuffer = StagingBufferResource.Buffer.get(),
                .DstBuffer = VertexBufferResource.Buffer.get()
            });
            
            BufferToBufferTransferRecorder.BufferTransfers[0].CopyRegions.push_back(
                {
                    .srcOffset = 0,
                    .dstOffset = 0,
                    .size = sizeof(VertexData),
                }
            );
            
            std::vector<VkBufferMemoryBarrier> BufferBarriers;
            BufferToBufferTransferRecorder.Resize(BufferBarriers);
            
            TransferCommandPool = lvkCommandPoolUtil::CreateCommandPool(DemoFramework.Device.get(),DemoFramework.GraphicsQueueFamilyIndex,VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
            lvkCommandPoolUtil TransferCommandPoolUtil(DemoFramework.Device.get(),TransferCommandPool.get());
            
            VkCommandBuffer TransferBuffer = TransferCommandPoolUtil.Allocate();
            {
                lvkRecordCmdBufferGuard TransferGuard(TransferBuffer,VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
                
                BufferToBufferTransferRecorder.RecordTransferOps(TransferBuffer);
                BufferToBufferTransferRecorder.RecordShaderAccessBarrier(BufferBarriers,TransferBuffer);
            }
            
            TransferTaskGraph.Submit(1,&TransferBuffer);
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
                        Viewport.width = Swapchain->GetExtent().width;
                        Viewport.height = Swapchain->GetExtent().height;
                        Viewport.minDepth = 0.0f;
                        Viewport.maxDepth = 1.0f;
                        
                        VkRect2D Scissor = {};
                        Scissor.offset = {0, 0};
                        Scissor.extent.width = Swapchain->GetExtent().width;
                        Scissor.extent.height = Swapchain->GetExtent().height;
                        
                        vkCmdSetViewport(CommandBuffers[i],0,1,&Viewport);
                        vkCmdSetScissor(CommandBuffers[i],0,1,&Scissor);
                        
                        vkCmdBindPipeline(CommandBuffers[i],VK_PIPELINE_BIND_POINT_GRAPHICS,Pipeline.get());
                        
                        std::array<VkBuffer,1>     VertexBuffers       = {VertexBufferResource.Buffer.get()};
                        std::array<VkDeviceSize,1> VertexBufferOffsets = {0};
                        
                        vkCmdBindVertexBuffers(CommandBuffers[i],0,1,VertexBuffers.data(),VertexBufferOffsets.data());
                        
                        vkCmdDraw(CommandBuffers[i],3,1,0,0);
                    },
                    *Swapchain.get()
                )
            );
        }
        
        TaskGraph.Initialize(DemoFramework.Device.get(),DemoFramework.GraphicsQueue,DemoFramework.PresentQueue);
    }
    
    ~lvkVertexStagingBufferApp()
    {
        vkDeviceWaitIdle(DemoFramework.Device.get());
    }
};

const bool IsResizable = true;

std::unique_ptr<IVKSampleApp> CreateSampleApp(IVKWindow &window)
{
    return std::unique_ptr<IVKSampleApp>(new lvkVertexStagingBufferApp(window));
}
