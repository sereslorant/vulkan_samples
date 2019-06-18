#ifndef LVK_IMAGE_VIEW_H
#define LVK_IMAGE_VIEW_H

#include <vulkan/vulkan.h>

#include <memory>

//TMP!!!!!!!!!!!!!!!!
#include <iostream>
//TMP!!!!!!!!!!!!!!!!

class lvkImageViewDeleter
{
private:
    VkDevice Device;

public:

    void operator()(VkImageView image_view)
    {
        if(Device != VK_NULL_HANDLE)
        {
            std::cout << "Deleting image view" << std::endl;
            vkDestroyImageView(Device,image_view,nullptr);
        }
    }

    lvkImageViewDeleter(VkDevice device = VK_NULL_HANDLE)
        :Device(device)
    {}
};

using lvkImageView = std::unique_ptr<VkImageView_T,lvkImageViewDeleter>;

#endif // LVK_IMAGE_VIEW_H
