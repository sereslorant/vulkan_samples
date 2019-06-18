#ifndef LVK_SAMPLER_H
#define LVK_SAMPLER_H

#include <vulkan/vulkan.h>

#include <memory>

//TMP!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#include <iostream>
//TMP!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

class lvkSamplerDeleter
{
private:
    VkDevice Device;
        
public:
    
    void operator()(VkSampler sampler)
    {
        if(Device != VK_NULL_HANDLE)
        {
            std::cout << "Deleting sampler" << std::endl;
            vkDestroySampler(Device,sampler,nullptr);
        }
    }
    
    lvkSamplerDeleter(VkDevice device = VK_NULL_HANDLE)
        :Device(device)
    {}
};

using lvkSampler = std::unique_ptr<VkSampler_T,lvkSamplerDeleter>;

#endif // LVK_SAMPLER_H
