#ifndef LVK_IMAGE_H
#define LVK_IMAGE_H

#include <vulkan/vulkan.h>

#include <memory>

//TMP!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#include <iostream>
//TMP!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

class lvkImageDeleter
{
private:
    VkDevice Device;
        
public:
    
    void operator()(VkImage image)
    {
        if(Device != VK_NULL_HANDLE)
        {
            std::cout << "Deleting image" << std::endl;
            vkDestroyImage(Device,image,nullptr);
        }
    }
    
    lvkImageDeleter(VkDevice device = VK_NULL_HANDLE)
        :Device(device)
    {}
};

using lvkImage = std::unique_ptr<VkImage_T,lvkImageDeleter>;

#endif // LVK_IMAGE_H
