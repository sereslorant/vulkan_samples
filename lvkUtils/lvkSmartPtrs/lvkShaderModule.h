#ifndef LVK_SHADER_MODULE_H
#define LVK_SHADER_MODULE_H

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

//TMP!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#include <iostream>
//TMP!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

class lvkShaderModuleDeleter
{
private:
    VkDevice Device;
    
public:
    
    void operator()(VkShaderModule shader)
    {
        if(Device != VK_NULL_HANDLE)
        {
            std::cout << "Deleting shader module" << std::endl;
            vkDestroyShaderModule(Device,shader,nullptr);
        }
    }
    
    lvkShaderModuleDeleter(VkDevice device = VK_NULL_HANDLE)
        :Device(device)
    {}
};

using lvkShaderModule = std::unique_ptr<VkShaderModule_T,lvkShaderModuleDeleter>;

#endif // LVK_SHADER_MODULE_H
