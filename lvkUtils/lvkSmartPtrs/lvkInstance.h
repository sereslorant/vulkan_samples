#ifndef LVK_INSTANCE_H
#define LVK_INSTANCE_H

#include <vulkan/vulkan.h>

#include <memory>

//TMP!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#include <iostream>
//TMP!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

class lvkInstanceDeleter
{
public:
    
    void operator()(VkInstance instance)
    {
        std::cout << "Deleting instance" << std::endl;
        vkDestroyInstance(instance,nullptr);
    }
};

using lvkInstance = std::unique_ptr<VkInstance_T,lvkInstanceDeleter>;

#endif // LVK_INSTANCE_H
