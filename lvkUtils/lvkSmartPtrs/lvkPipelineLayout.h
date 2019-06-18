#ifndef LVK_PIPELINE_LAYOUT_H
#define LVK_PIPELINE_LAYOUT_H

#include <vulkan/vulkan.h>

#include <memory>

//TMP!!!!!!!!!!!!!!!!
#include <iostream>
//TMP!!!!!!!!!!!!!!!!

class lvkPipelineLayoutDeleter
{
private:
    VkDevice Device;
    
public:
    
    void operator()(VkPipelineLayout pipeline_layout)
    {
        if(Device != VK_NULL_HANDLE)
        {
            std::cout << "Deleting pipeline layout" << std::endl;
            vkDestroyPipelineLayout(Device,pipeline_layout,nullptr);
        }
    }
    
    lvkPipelineLayoutDeleter(VkDevice device = VK_NULL_HANDLE)
        :Device(device)
    {}
};

using lvkPipelineLayout = std::unique_ptr<VkPipelineLayout_T,lvkPipelineLayoutDeleter>;

#endif // LVK_PIPELINE_LAYOUT_H
