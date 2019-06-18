#ifndef LVK_DESCRIPTOR_SET_LAYOUT_H
#define LVK_DESCRIPTOR_SET_LAYOUT_H

#include <vulkan/vulkan.h>

#include <memory>

//TMP!!!!!!!!!!!!!!!!
#include <iostream>
//TMP!!!!!!!!!!!!!!!!

class lvkDescriptorSetLayoutDeleter
{
private:
    VkDevice Device;
    
public:
    
    void operator()(VkDescriptorSetLayout descriptor_set_layout)
    {
        if(Device != VK_NULL_HANDLE)
        {
            std::cout << "Deleting descriptor set layout" << std::endl;
            vkDestroyDescriptorSetLayout(Device,descriptor_set_layout,nullptr);
        }
    }
    
    lvkDescriptorSetLayoutDeleter(VkDevice device = VK_NULL_HANDLE)
        :Device(device)
    {}
};

using lvkDescriptorSetLayout = std::unique_ptr<VkDescriptorSetLayout_T,lvkDescriptorSetLayoutDeleter>;

#endif // LVK_DESCRIPTOR_SET_LAYOUT_H
