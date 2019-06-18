#ifndef LVK_PIPELINE_H
#define LVK_PIPELINE_H

#include <vulkan/vulkan.h>

#include <memory>

//TMP!!!!!!!!!!!!!!!!
#include <iostream>
//TMP!!!!!!!!!!!!!!!!

class lvkPipelineDeleter
{
private:
    VkDevice Device;
    
public:
    
    void operator()(VkPipeline graphics_pipeline)
    {
        if(Device != VK_NULL_HANDLE)
        {
            std::cout << "Deleting pipeline" << std::endl;
            vkDestroyPipeline(Device,graphics_pipeline,nullptr);
        }
    }
    
    lvkPipelineDeleter(VkDevice device = VK_NULL_HANDLE)
        :Device(device)
    {}
};

using lvkPipeline = std::unique_ptr<VkPipeline_T,lvkPipelineDeleter>;

#endif // LVK_PIPELINE_H

