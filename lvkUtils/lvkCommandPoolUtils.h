#ifndef LVK_COMMAND_POOL_UTILS_H
#define LVK_COMMAND_POOL_UTILS_H

#include <lvkUtils/lvkSmartPtrs/lvkCommandPool.h>

#include <vector>

class lvkCommandPoolUtil
{
private:
    VkDevice      Device;
    VkCommandPool CommandPool;
    
public:
    
    static lvkCommandPool CreateCommandPool(VkDevice device,std::uint32_t queue_family,VkCommandPoolCreateFlags flags = 0x0)
    {
        VkCommandPoolCreateInfo CommandPoolCreateInfo =
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = flags,
            .queueFamilyIndex = queue_family,
        };
        
        VkCommandPool CommandPool;
        std::cout << "Creating command pool" << std::endl;
        if(vkCreateCommandPool(device,&CommandPoolCreateInfo,nullptr,&CommandPool) != VK_SUCCESS)
        {
            std::cout << "Failed to create command pool!" << std::endl;
        }
        return lvkCommandPool{CommandPool,{device}};
    }
    
    VkCommandBuffer Allocate(VkCommandBufferLevel buffer_level = VK_COMMAND_BUFFER_LEVEL_PRIMARY)
    {
        VkCommandBufferAllocateInfo CommandBufferAllocateInfo =
        {
            .sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext       = nullptr,
            .commandPool = CommandPool,
            .level       = buffer_level,
            .commandBufferCount = 1,
        };
        
        VkCommandBuffer CommandBuffer;
        std::cout << "Creating command buffer" << std::endl;
        if(vkAllocateCommandBuffers(Device,&CommandBufferAllocateInfo,&CommandBuffer) != VK_SUCCESS)
        {
            std::cout << "failed to allocate command buffers!" << std::endl;
        }
        
        return CommandBuffer;
    }
    
    std::vector<VkCommandBuffer> Allocate(std::uint32_t cmd_buffer_count,VkCommandBufferLevel buffer_level = VK_COMMAND_BUFFER_LEVEL_PRIMARY)
    {
        VkCommandBufferAllocateInfo CommandBufferAllocateInfo =
        {
            .sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext       = nullptr,
            .commandPool = CommandPool,
            .level       = buffer_level,
            .commandBufferCount = cmd_buffer_count,
        };
        
        std::vector<VkCommandBuffer> CommandBuffers(cmd_buffer_count);
        std::cout << "Creating command buffer" << std::endl;
        if(vkAllocateCommandBuffers(Device,&CommandBufferAllocateInfo,CommandBuffers.data()) != VK_SUCCESS)
        {
            std::cout << "failed to allocate command buffers!" << std::endl;
            return {};
        }
        return CommandBuffers;
    }
    
    void Reset()
    {
        vkResetCommandPool(Device,CommandPool,0x0);
    }
    
    void operator=(const lvkCommandPoolUtil &other)
    {
        Device      = other.Device;
        CommandPool = other.CommandPool;
    }
    
    lvkCommandPoolUtil(VkDevice device = VK_NULL_HANDLE,VkCommandPool command_pool = VK_NULL_HANDLE)
        :Device(device),CommandPool(command_pool)
    {}
};

#endif // LVK_COMMAND_POOL_UTILS_H
