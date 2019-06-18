#ifndef LVK_FRAMEBUFFER_H
#define LVK_FRAMEBUFFER_H

#include <vulkan/vulkan.h>

#include <memory>

//TMP!!!!!!!!!!!!!!!!
#include <iostream>
//TMP!!!!!!!!!!!!!!!!

class lvkFramebufferDeleter
{
private:
    VkDevice Device;

public:

    void operator()(VkFramebuffer framebuffer)
    {
        if(Device != VK_NULL_HANDLE)
        {
            std::cout << "Deleting framebuffer" << std::endl;
            vkDestroyFramebuffer(Device,framebuffer,nullptr);
        }
    }

    lvkFramebufferDeleter(VkDevice device = VK_NULL_HANDLE)
        :Device(device)
    {}
};

using lvkFramebuffer = std::unique_ptr<VkFramebuffer_T,lvkFramebufferDeleter>;

#endif // LVK_FRAMEBUFFER_H
