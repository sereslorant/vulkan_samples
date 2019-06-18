#ifndef LVK_SEMAPHORE_H
#define LVK_SEMAPHORE_H

#include <vulkan/vulkan.h>

#include <memory>

//TMP!!!!!!!!!!!!!!!!
#include <iostream>
//TMP!!!!!!!!!!!!!!!!

class lvkSemaphoreDeleter
{
private:
    VkDevice Device;

public:

    void operator()(VkSemaphore semaphore)
    {
        if(Device != VK_NULL_HANDLE)
        {
            std::cout << "Deleting semaphore" << std::endl;
            vkDestroySemaphore(Device,semaphore,nullptr);
        }
    }

    lvkSemaphoreDeleter(VkDevice device = VK_NULL_HANDLE)
        :Device(device)
    {}
};

using lvkSemaphore = std::unique_ptr<VkSemaphore_T,lvkSemaphoreDeleter>;

#endif // LVK_SEMAPHORE_H
