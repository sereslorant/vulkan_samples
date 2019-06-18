#ifndef LVK_RENDER_PASS_H
#define LVK_RENDER_PASS_H

#include <vulkan/vulkan.h>

#include <memory>

//TMP!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#include <iostream>
//TMP!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

class lvkRenderPassDeleter
{
private:
    VkDevice Device;

public:

    void operator()(VkRenderPass render_pass)
    {
        if(Device != VK_NULL_HANDLE)
        {
            std::cout << "Deleting render pass" << std::endl;
            vkDestroyRenderPass(Device,render_pass,nullptr);
        }
    }

    lvkRenderPassDeleter(VkDevice device = VK_NULL_HANDLE)
        :Device(device)
    {}
};

using lvkRenderPass = std::unique_ptr<VkRenderPass_T,lvkRenderPassDeleter>;

#endif // LVK_RENDER_PASS_H
