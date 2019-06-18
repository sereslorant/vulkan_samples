#ifndef LVK_DESCRIPTOR_POOL_H
#define LVK_DESCRIPTOR_POOL_H

#include <vulkan/vulkan.h>

#include <memory>

//TMP!!!!!!!!!!!!!!!!
#include <iostream>
//TMP!!!!!!!!!!!!!!!!

class lvkDescriptorPoolDeleter
{
private:
    VkDevice Device;
    
public:
    
    void operator()(VkDescriptorPool descriptor_pool)
    {
        if(Device != VK_NULL_HANDLE)
        {
            std::cout << "Deleting descriptor pool" << std::endl;
            vkDestroyDescriptorPool(Device,descriptor_pool,nullptr);
        }
    }
    
    lvkDescriptorPoolDeleter(VkDevice device = VK_NULL_HANDLE)
        :Device(device)
    {}
};

using lvkDescriptorPool = std::unique_ptr<VkDescriptorPool_T,lvkDescriptorPoolDeleter>;

#endif // LVK_DESCRIPTOR_POOL_H
