#ifndef LVK_SURFACE_KHR
#define LVK_SURFACE_KHR

#include <vulkan/vulkan.h>

#include <memory>

//TMP!!!!!!!!!!!!!!!!
#include <iostream>
//TMP!!!!!!!!!!!!!!!!

class lvkSurfaceKHRDeleter
{
private:
    VkInstance Instance;

public:

    void operator()(VkSurfaceKHR surface)
    {
        if(Instance != VK_NULL_HANDLE)
        {
            std::cout << "Deleting surface" << std::endl;
            vkDestroySurfaceKHR(Instance,surface,nullptr);
        }
    }

    lvkSurfaceKHRDeleter(VkInstance instance = VK_NULL_HANDLE)
        :Instance(instance)
    {}
};

using lvkSurfaceKHR = std::unique_ptr<VkSurfaceKHR_T,lvkSurfaceKHRDeleter>;

#endif // LVK_SURFACE_KHR
