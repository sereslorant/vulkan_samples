#ifndef LVK_BUFFER_H
#define LVK_BUFFER_H

#include <vulkan/vulkan.h>

#include <memory>

//TMP!!!!!!!!!!!!!!!!
#include <iostream>
//TMP!!!!!!!!!!!!!!!!

class lvkBufferDeleter
{
private:
    VkDevice Device;
    
public:
    
    void operator()(VkBuffer buffer)
    {
        if(Device != VK_NULL_HANDLE)
        {
            std::cout << "Deleting buffer" << std::endl;
            vkDestroyBuffer(Device,buffer,nullptr);
        }
    }
    
    lvkBufferDeleter(VkDevice device = VK_NULL_HANDLE)
        :Device(device)
    {}
};

using lvkBuffer = std::unique_ptr<VkBuffer_T,lvkBufferDeleter>;

#endif // LVK_BUFFER_H
