#ifndef LVK_RENDERER_H
#define LVK_RENDERER_H

#include <lvkUtils/lvkSmartPtrs/lvkRenderPass.h>

#include <vector>

/*
 * Frame Graph builder
 */

struct lvkRenderPassTopology
{
    struct lvkSubpassReferences
    {
        std::vector<VkAttachmentReference>     ColorAttachments;
        std::unique_ptr<VkAttachmentReference> DepthAttachment;
        
        void SetAttachments(VkSubpassDescription &subpass)
        {
            if(ColorAttachments.size() > 0)
            {
                subpass.colorAttachmentCount = ColorAttachments.size();
                subpass.pColorAttachments    = ColorAttachments.data();
            }
            
            if(DepthAttachment != nullptr)
            {
                subpass.pDepthStencilAttachment = DepthAttachment.get();
            }
        }
    };
    
    /*
     * Frame graph nodes
     */
    
    std::vector<VkAttachmentDescription> Attachments;
    std::vector<VkSubpassDescription>    Subpasses;
    
    /*
     * Frame graph edges
     */
    
    std::vector<lvkSubpassReferences> SubpassReferences;
    std::vector<VkSubpassDependency>  SubpassDependencies;
    
    void ApplyToRenderPass(VkRenderPassCreateInfo &render_pass_create_info)
    {
        for(std::size_t i=0;i < Subpasses.size();i++)
            {SubpassReferences[i].SetAttachments(Subpasses[i]);}
        
        render_pass_create_info.attachmentCount = Attachments.size();
        render_pass_create_info.pAttachments    = Attachments.data();
        
        render_pass_create_info.subpassCount = Subpasses.size();
        render_pass_create_info.pSubpasses   = Subpasses.data();
        
        render_pass_create_info.dependencyCount = SubpassDependencies.size();
        render_pass_create_info.pDependencies   = SubpassDependencies.data();
    }
};

class lvkAttachmentBuilder
{
private:
    lvkRenderPassTopology &RenderPassTopology;
    std::uint32_t AttachmentId;
    
public:
    
    void SetFormat(VkFormat format)
    {
        RenderPassTopology.Attachments[AttachmentId].format = format;
    }
    
    std::uint32_t Build()
    {
        return AttachmentId;
    }
    
    lvkAttachmentBuilder(lvkRenderPassTopology &render_pass_topology,std::uint32_t attachment_id)
        :RenderPassTopology(render_pass_topology),AttachmentId(attachment_id)
    {
        RenderPassTopology.Attachments[AttachmentId] = LVK_PRESENT_CLEAR_ATTACHMENT_DESCRIPTION;
    }
};

class lvkSubpassBuilder
{
private:
    lvkRenderPassTopology &RenderPassTopology;
    std::uint32_t SubpassId;
    
public:
    
    void AddColorAttachment(std::uint32_t attachment_id)
    {
        RenderPassTopology.SubpassReferences[SubpassId].ColorAttachments.push_back(LVK_DEFAULT_COLOR_ATTACHMENT_REFERENCE);
        RenderPassTopology.SubpassReferences[SubpassId].ColorAttachments.back().attachment = attachment_id;
    }
    
    void SetDepthAttachment(std::uint32_t attachment_id)
    {
        if(RenderPassTopology.SubpassReferences[SubpassId].DepthAttachment == nullptr)
        {
            RenderPassTopology.SubpassReferences[SubpassId].DepthAttachment = std::unique_ptr<VkAttachmentReference>(
                new VkAttachmentReference(LVK_DEFAULT_DEPTH_ATTACHMENT_REFERENCE)
            );
        }
        
        RenderPassTopology.SubpassReferences[SubpassId].DepthAttachment->attachment = attachment_id;
    }
    
    void AddDependency(std::uint32_t previous_subpass_id)
    {
        RenderPassTopology.SubpassDependencies.push_back(LVK_DEFAULT_PREVIOUS_EXTERNAL_SUBPASS_DEPENDENCY);
        RenderPassTopology.SubpassDependencies.back().srcSubpass = previous_subpass_id;
        RenderPassTopology.SubpassDependencies.back().dstSubpass = SubpassId;
    }
    
    std::uint32_t Build()
    {
        return SubpassId;
    }
    
    lvkSubpassBuilder(lvkRenderPassTopology &render_pass_topology,std::uint32_t subpass_id)
        :RenderPassTopology(render_pass_topology),SubpassId(subpass_id)
    {
        RenderPassTopology.Subpasses[SubpassId] = LVK_EMPTY_SUBPASS_DESCRIPTION;
    }
};

class lvkRenderPassBuilder
{
private:
    lvkRenderPassTopology RenderPassTopology;
    VkDevice Device;
    
public:
    
    lvkAttachmentBuilder CreateAttachment()
    {
        std::uint32_t AttachmentId = RenderPassTopology.Attachments.size();
        RenderPassTopology.Attachments.push_back({});
        return {RenderPassTopology,AttachmentId};
    }
    
    lvkSubpassBuilder CreateSubpass()
    {
        std::uint32_t SubpassId = RenderPassTopology.Subpasses.size();
        RenderPassTopology.Subpasses.push_back({});
        RenderPassTopology.SubpassReferences.push_back({});
        return {RenderPassTopology,SubpassId};
    }
    
    lvkRenderPass Build()
    {
        VkRenderPassCreateInfo RenderPassCreateInfo = LVK_EMPTY_RENDER_PASS_CREATE_INFO;
        RenderPassTopology.ApplyToRenderPass(RenderPassCreateInfo);
        
        std::cout << "Creating render pass" << std::endl;
        VkRenderPass NewRenderPass;
        if(vkCreateRenderPass(Device,&RenderPassCreateInfo,nullptr,&NewRenderPass) != VK_SUCCESS)
        {
            std::cout << "Error while creating render pass" << std::endl;
        }
        
        return lvkRenderPass{NewRenderPass,{Device}};
    }
    
    lvkRenderPassBuilder(VkDevice device)
        :Device(device)
    {}
};

/*
 * GPU Submission graph
 */

class lvkGpuSubmission
{
protected:
    std::vector<VkSemaphore> SignalSemaphores;
    std::vector<VkSemaphore> WaitSemaphores;
    
public:
    
    static VkSemaphore CreateSemaphore(VkDevice device)
    {
        VkSemaphoreCreateInfo SemaphoreCreateInfo = {};
        SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        std::cout << "Creating semaphore" << std::endl;
        VkSemaphore Semaphore;
        if(vkCreateSemaphore(device,&SemaphoreCreateInfo,nullptr,&Semaphore) != VK_SUCCESS)
        {
            std::cout << "failed to create semaphore!" << std::endl;
        }

        return Semaphore;
    }
    
    static lvkFence CreateFence(VkDevice device,VkFenceCreateFlags flags = 0x0)
    {
        VkFenceCreateInfo FenceCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = flags
        };

        std::cout << "Creating fence" << std::endl;
        VkFence Fence;
        if(vkCreateFence(device,&FenceCreateInfo,nullptr,&Fence) != VK_SUCCESS)
        {
            std::cout << "Failed to create fence" << std::endl;
        }

        return lvkFence{Fence,{device}};
    }
    
    void AddSignalSemaphore(VkSemaphore new_semaphore)
    {
        SignalSemaphores.push_back(new_semaphore);
    }
    
    lvkGpuSubmission(std::size_t signal_count = 0,std::size_t dependency_count = 0)
    {
        SignalSemaphores.reserve(signal_count);
        WaitSemaphores.reserve(dependency_count);
    }
    
    virtual ~lvkGpuSubmission()
    {}
};

class lvkGpuCommandSubmitTask : public lvkGpuSubmission
{
private:
    VkQueue Queue;
    std::vector<VkPipelineStageFlags> WaitStages;
    
public:
    
    void AddDependency(VkSemaphore dependency_semaphore,VkPipelineStageFlags dependency_wait_stage)
    {
        WaitSemaphores.push_back(dependency_semaphore);
        WaitStages.push_back(dependency_wait_stage);
    }
    
    void Submit(std::uint32_t command_buffer_count,const VkCommandBuffer *command_buffers,VkFence fence = VK_NULL_HANDLE)
    {
        VkSubmitInfo SubmitInfo = {};
        SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        
        if(WaitSemaphores.size() > 0)
        {
            SubmitInfo.waitSemaphoreCount = WaitSemaphores.size();
            SubmitInfo.pWaitSemaphores    = WaitSemaphores.data();
            SubmitInfo.pWaitDstStageMask  = WaitStages.data();
        }
        
        SubmitInfo.commandBufferCount = command_buffer_count;
        SubmitInfo.pCommandBuffers    = command_buffers;
        
        if(SignalSemaphores.size() > 0)
        {
            SubmitInfo.signalSemaphoreCount = SignalSemaphores.size();
            SubmitInfo.pSignalSemaphores    = SignalSemaphores.data();
        }
        
        if(vkQueueSubmit(Queue,1,&SubmitInfo,fence) != VK_SUCCESS)
        {
            std::cerr << "failed to submit command buffer!" << std::endl;
        }
    }
    
    lvkGpuCommandSubmitTask(VkQueue queue = VK_NULL_HANDLE,std::size_t signal_count = 0,std::size_t dependency_count = 0)
        :lvkGpuSubmission(signal_count,dependency_count),Queue(queue)
    {
        WaitStages.reserve(dependency_count);
    }
    
    virtual ~lvkGpuCommandSubmitTask() override
    {}
};

class lvkGpuPresentSubmitTask : public lvkGpuSubmission
{
private:
    VkQueue Queue;
    
public:
    
    void AddDependency(VkSemaphore dependency_semaphore)
    {
        WaitSemaphores.push_back(dependency_semaphore);
    }
    
    void Submit(std::uint32_t swapchain_count,const VkSwapchainKHR *swapchain,const std::uint32_t *image_indices)
    {
        VkPresentInfoKHR PresentInfo = {};
        PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        
        if(WaitSemaphores.size() > 0)
        {
            PresentInfo.waitSemaphoreCount = WaitSemaphores.size();
            PresentInfo.pWaitSemaphores    = WaitSemaphores.data();
        }
        
        PresentInfo.swapchainCount = swapchain_count;
        PresentInfo.pSwapchains    = swapchain;
        PresentInfo.pImageIndices  = image_indices;
        
        PresentInfo.pResults = nullptr;
        
        if(vkQueuePresentKHR(Queue,&PresentInfo) != VK_SUCCESS)
        {
            std::cerr << "failed to queue present!" << std::endl;
        }
    }
    
    lvkGpuPresentSubmitTask(VkQueue queue = VK_NULL_HANDLE,std::size_t dependency_count = 0)
        :lvkGpuSubmission(0,dependency_count),Queue(queue)
    {}
    
    virtual ~lvkGpuPresentSubmitTask() override
    {}
};

#endif // LVK_RENDERER_H
