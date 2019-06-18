#ifndef LVK_FENCE_H
#define LVK_FENCE_H

#include <vulkan/vulkan.h>

#include <memory>

//TMP!!!!!!!!!!!!!!!!
#include <iostream>
//TMP!!!!!!!!!!!!!!!!

class lvkFenceDeleter
{
private:
    VkDevice Device;

public:

    void operator()(VkFence fence)
    {
        if(Device != VK_NULL_HANDLE)
        {
            std::cout << "Deleting fence" << std::endl;
            vkDestroyFence(Device,fence,nullptr);
        }
    }

    lvkFenceDeleter(VkDevice device = VK_NULL_HANDLE)
        :Device(device)
    {}
};

using lvkFence = std::unique_ptr<VkFence_T,lvkFenceDeleter>;

#endif // LVK_FENCE_H
