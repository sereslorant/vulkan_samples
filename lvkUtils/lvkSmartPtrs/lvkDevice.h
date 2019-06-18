#ifndef LVK_DEVICE_H
#define LVK_DEVICE_H

#include <vulkan/vulkan.h>

#include <memory>

//TMP!!!!!!!!!!!!!!!!
#include <iostream>
//TMP!!!!!!!!!!!!!!!!

class lvkDeviceDeleter
{
public:

    void operator()(VkDevice device)
    {
        std::cout << "Deleting device" << std::endl;
        vkDeviceWaitIdle(device);
        vkDestroyDevice(device,nullptr);
    }
};

using lvkDevice = std::unique_ptr<VkDevice_T,lvkDeviceDeleter>;

#endif // LVK_DEVICE_H
