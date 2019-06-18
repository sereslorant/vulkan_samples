#ifndef LVK_SWAPCHAIN_KHR
#define LVK_SWAPCHAIN_KHR

#include <vulkan/vulkan.h>

#include <memory>

//TMP!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#include <iostream>
//TMP!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

class lvkSwapchainDeleter
{
private:
    VkDevice Device;

public:

    void operator()(VkSwapchainKHR swapchain)
    {
        if(Device != VK_NULL_HANDLE)
        {
            std::cout << "Deleting swapchain" << std::endl;
            vkDestroySwapchainKHR(Device,swapchain,nullptr);
        }
    }

    lvkSwapchainDeleter(VkDevice device = VK_NULL_HANDLE)
        :Device(device)
    {}
};

using lvkSwapchainKHR = std::unique_ptr<VkSwapchainKHR_T,lvkSwapchainDeleter>;

#endif // LVK_SWAPCHAIN_KHR
