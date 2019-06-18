#ifndef LVK_COMMAND_POOL_H
#define LVK_COMMAND_POOL_H

#include <vulkan/vulkan.h>

#include <memory>

//TMP!!!!!!!!!!!!!!!!
#include <iostream>
//TMP!!!!!!!!!!!!!!!!

class lvkCommandPoolDeleter
{
private:
    VkDevice Device;
    
public:
    
    void operator()(VkCommandPool command_pool)
    {
        if(Device != VK_NULL_HANDLE)
        {
            std::cout << "Deleting command pool" << std::endl;
            vkDestroyCommandPool(Device,command_pool,nullptr);
        }
    }
    
    lvkCommandPoolDeleter(VkDevice device = VK_NULL_HANDLE)
        :Device(device)
    {}
};

using lvkCommandPool = std::unique_ptr<VkCommandPool_T,lvkCommandPoolDeleter>;

#endif // LVK_COMMAND_POOL_H
