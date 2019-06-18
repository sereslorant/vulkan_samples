
#include <lvkSampleApps/lvkDemoUtils.h>

#include <lvkUtils/lvkShaderModuleLibrary.h>

#include <lvkRenderer/lvkResourceHierarchy.h>

class lvkMeshGroupApp : public IVKSampleApp
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
    lvkBufferResource IndexBufferResource;
    
    lvkMeshGroupVertexData MeshVertexBuffers;
    lvkMeshGroupIndexData  MeshIndexBuffer;
    lrMeshGroup            MeshGroup;
    
    std::vector<VkCommandBuffer> CommandBuffers;
    std::unique_ptr<lvkDemoCommandPool> CommandPoolResetObserver;
    std::vector<std::unique_ptr<lvkDemoBufferRecorder> > CommandRecorders;
    
    lvkDemoTaskGraph TaskGraph;
    
public:
    
    virtual void Loop() override
    {
        TaskGraph.Submit(DemoFramework.Device.get(),*Swapchain.get(),CommandBuffers.data());
    }
    
    lvkMeshGroupApp(IVKWindow &window)
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
        
        constexpr unsigned int NUM_VERTICES = 7;
        constexpr unsigned int NUM_COMPONENTS = 2;
        float VertexData[NUM_VERTICES*NUM_COMPONENTS] = {-0.125,-0.125,-0.125,0.125,0.125,0.125,0.125,-0.125,0.2,0.0,0.4,0.0,0.4,0.2};

        constexpr unsigned int NUM_INDICES = 6;
        std::uint32_t IndexData[NUM_INDICES] = {0,1,2,0,3,2};
        
        lvkMemoryUtility MemoryUtility(DemoFramework.Device.get(),DemoFramework.PhysicalDevice);
        std::unique_ptr<lvkMemoryAllocProcElement> Allocator = MemoryUtility.CreateAllocator(LVK_TRANSFER_MEMORY_PROPERTIES);
        
        lvkResourceFactory BufferFactory(
            DemoFramework.Device.get(),
            std::move(Allocator)
        );
        
        VertexBufferResource = BufferFactory.CreateBuffer(sizeof(VertexData),VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        {
            lvkMappedMemory MappedMemory = VertexBufferResource.Allocation.Map();
            lMemoryView FullMemoryView = MappedMemory.GetView();
            
            FullMemoryView.MemCpySec(VertexData,sizeof(VertexData));
        }
        
        IndexBufferResource = BufferFactory.CreateBuffer(sizeof(IndexData),VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
        {
            lvkMappedMemory MappedMemory = IndexBufferResource.Allocation.Map();
            lMemoryView FullMemoryView = MappedMemory.GetView();
            
            FullMemoryView.MemCpySec(IndexData,sizeof(IndexData));
        }
        
        MeshVertexBuffers = lvkMeshGroupVertexData{1,{{VertexBufferResource.Buffer.get()}},{{0}}};
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
                        
                        MeshVertexBuffers.BindVertexBuffers(CommandBuffers[i]);
                        MeshIndexBuffer.BindIndexBuffer(CommandBuffers[i]);
                        
                        vkCmdDrawIndexed(
                            CommandBuffers[i],
                            MeshGroup.IndexedSubMeshes[0].IndexCount,
                            1,
                            MeshGroup.IndexedSubMeshes[0].FirstIndex,
                            MeshGroup.IndexedSubMeshes[0].BaseVertex,
                            0
                        );
                        
                        vkCmdDraw(
                            CommandBuffers[i],
                            MeshGroup.SubMeshes[0].VertexCount,
                            1,
                            MeshGroup.SubMeshes[0].BaseVertex,
                            0
                        );
                    },
                    *Swapchain.get()
                )
            );
        }
        
        TaskGraph.Initialize(DemoFramework.Device.get(),DemoFramework.GraphicsQueue,DemoFramework.PresentQueue);
    }
    
    ~lvkMeshGroupApp()
    {
        vkDeviceWaitIdle(DemoFramework.Device.get());
    }
};

const bool IsResizable = true;

std::unique_ptr<IVKSampleApp> CreateSampleApp(IVKWindow &window)
{
    return std::unique_ptr<IVKSampleApp>(new lvkMeshGroupApp(window));
}
