
#include <lvkSampleApps/lvkDemoUtils.h>

#include <lvkUtils/lvkShaderModuleLibrary.h>

class lvkDemoNonDynamicPipeline : public lvkSwapchain::liObserver
{
private:
    VkDevice Device;
    
    lvkDemoShaderStages           DemoShaderStages;
    VkGraphicsPipelineCreateInfo  PipelineCreateInfo = LVK_EMPTY_PIPELINE_CREATE_INFO;
    
    lvkSwapchain &Swapchain;
    
    lvkPipeline Pipeline;
    
public:
    
    VkPipeline GetPipeline()
    {
        return Pipeline.get();
    }
    
    virtual void Destroyed() override
    {
        Pipeline = nullptr;
    }
    
    virtual void Recreated() override
    {
        lvkPipelineFixedFunctionState FixedFunctionState;
        lvkDemoColorBlendAttachments  DemoColorBlendAttachments;
        
        FixedFunctionState.SetViewport(0.0,0.0,Swapchain.GetExtent().width,Swapchain.GetExtent().height);
        FixedFunctionState.SetScissor(0,0,Swapchain.GetExtent().width,Swapchain.GetExtent().height);
        
        DemoShaderStages.AssignToPipelineCreateInfo(PipelineCreateInfo);
        FixedFunctionState.AssignToPipelineCreateInfo(PipelineCreateInfo);
        DemoColorBlendAttachments.AssignToPipelineCreateInfo(PipelineCreateInfo);
        
        std::cout << "Creating graphics pipeline" << std::endl;
        VkPipeline NewPipeline;
        if(vkCreateGraphicsPipelines(Device,VK_NULL_HANDLE,1,&PipelineCreateInfo,nullptr,&NewPipeline) != VK_SUCCESS)
        {
            std::cout << "Error while creating pipeline" << std::endl;
        }
        
        Pipeline = lvkPipeline{NewPipeline,{Device}};
    }
    
    lvkDemoNonDynamicPipeline(VkDevice device,VkShaderModule vertex_shader,VkShaderModule fragment_shader,VkRenderPass render_pass,std::uint32_t subpass,VkPipelineLayout pipeline_layout,lvkSwapchain &swapchain)
        :Device(device),Swapchain(swapchain)
    {
        DemoShaderStages.SetShaders(
                vertex_shader,
                fragment_shader
            );
        
        PipelineCreateInfo.renderPass = render_pass;
        PipelineCreateInfo.subpass    = subpass;
        PipelineCreateInfo.layout     = pipeline_layout;
        
        Recreated();
        
        Swapchain.AddObserver(this);
    }
    
    ~lvkDemoNonDynamicPipeline()
    {
        Swapchain.RemoveObserver(this);
    }
};

class lvkTriangleApp : public IVKSampleApp
{
private:
    lvkDemoFramework DemoFramework;
    
    lvkShaderModuleLibrary ShaderModuleLibrary;
    
    std::unique_ptr<lvkSwapchain> Swapchain;
    lvkRenderPass RenderPass;
    std::unique_ptr<lvkDemoSwapchainFramebuffers> DemoSwapchainFramebuffers;
    
    lvkPipelineLayout PipelineLayout;
    std::unique_ptr<lvkDemoNonDynamicPipeline> Pipeline;
    
    std::vector<VkCommandBuffer> CommandBuffers;
    std::unique_ptr<lvkDemoCommandPool> CommandPoolResetObserver;
    std::vector<std::unique_ptr<lvkDemoBufferRecorder> > CommandRecorders;
    
    lvkDemoTaskGraph TaskGraph;
    
public:
    
    virtual void Loop() override
    {
        TaskGraph.Submit(DemoFramework.Device.get(),*Swapchain.get(),CommandBuffers.data());
    }
    
    lvkTriangleApp(IVKWindow &window)
        :DemoFramework(window)
    {
        ShaderModuleLibrary = {DemoFramework.Device.get()};
        
        Swapchain = std::unique_ptr<lvkSwapchain>(new lvkSwapchain(DemoFramework.Device.get(),DemoFramework.PhysicalDevice,DemoFramework.Surface.get()));
        RenderPass = DemoFramework.CreateSimpleRenderPass(DemoFramework.Device.get(),Swapchain->GetFormat());
        DemoSwapchainFramebuffers = std::unique_ptr<lvkDemoSwapchainFramebuffers>(new lvkDemoSwapchainFramebuffers(DemoFramework.Device.get(),RenderPass.get(),*Swapchain.get()));
        
        PipelineLayout = DemoFramework.CreateEmptyPipelineLayout();
        
        Pipeline = std::unique_ptr<lvkDemoNonDynamicPipeline>(
            new lvkDemoNonDynamicPipeline(
                    DemoFramework.Device.get(),
                    ShaderModuleLibrary.GetShaderModule("../Shaders/HardcodedVertexShader.spv"),
                    ShaderModuleLibrary.GetShaderModule("../Shaders/FragmentShader.spv"),
                    RenderPass.get(),
                    0,
                    PipelineLayout.get(),
                    *Swapchain.get()
                )
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
                        
                        vkCmdBindPipeline(CommandBuffers[i],VK_PIPELINE_BIND_POINT_GRAPHICS,Pipeline->GetPipeline());
                        
                        vkCmdDraw(CommandBuffers[i],3,1,0,0);
                    },
                    *Swapchain.get()
                )
            );
        }
        
        TaskGraph.Initialize(DemoFramework.Device.get(),DemoFramework.GraphicsQueue,DemoFramework.PresentQueue);
    }
    
    ~lvkTriangleApp()
    {
        vkDeviceWaitIdle(DemoFramework.Device.get());
    }
};

const bool IsResizable = true;

std::unique_ptr<IVKSampleApp> CreateSampleApp(IVKWindow &window)
{
    return std::unique_ptr<IVKSampleApp>(new lvkTriangleApp(window));
}
