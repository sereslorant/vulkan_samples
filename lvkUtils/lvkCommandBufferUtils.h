#ifndef LVK_COMMAND_BUFFER_UTILS_H
#define LVK_COMMAND_BUFFER_UTILS_H

#include <vulkan/vulkan.h>

//TMP!!!!!!!!!!!!!!!!
#include <iostream>
//TMP!!!!!!!!!!!!!!!!

class lvkRecordCmdBufferGuard
{
private:
    VkCommandBuffer CommandBuffer;
    
public:
    
    lvkRecordCmdBufferGuard(VkCommandBuffer command_buffer,
                            VkCommandBufferUsageFlags flags,
                            const VkCommandBufferInheritanceInfo *inheriance_info = nullptr
                           )
        :CommandBuffer(command_buffer)
    {
        VkCommandBufferBeginInfo BeginInfo =
        {
            .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext            = nullptr,
            .flags            = flags,
            .pInheritanceInfo = inheriance_info,
        };
        
        vkBeginCommandBuffer(CommandBuffer,&BeginInfo);
    }
    
    ~lvkRecordCmdBufferGuard()
    {
        if(vkEndCommandBuffer(CommandBuffer) != VK_SUCCESS)
        {
            std::cout << "failed to record command buffer!" << std::endl;
        }
    }
};

#endif // LVK_COMMAND_BUFFER_UTILS_H
