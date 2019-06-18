#ifndef LVK_TRANSFER_RECORDER_H
#define LVK_TRANSFER_RECORDER_H

#include <cinttypes>
#include <vector>

#include <vulkan/vulkan.h>

/*
 * Transfer command recorders
 */

struct lvkBufferToImageTransferBatch
{
    VkBuffer Buffer;
    VkImage  Image;
    std::uint32_t Width;
    std::uint32_t Height;
    
    std::vector<VkBufferImageCopy> CopyRegions;
    
    void AddTransfer(VkDeviceSize buffer_offset,std::uint32_t first_image,std::uint32_t image_count)
    {
        CopyRegions.push_back(
            {
                .bufferOffset      = buffer_offset,
                .bufferRowLength   = 0,
                .bufferImageHeight = 0,
                
                .imageSubresource = {
                    .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel       = 0,
                    .baseArrayLayer = first_image,
                    .layerCount     = image_count
                },
                
                .imageOffset = {0,0,0},
                .imageExtent = {Width,Height,1},
            }
        );
    }
    
    void RecordTransferOps(VkCommandBuffer command_buffer)
    {
        vkCmdCopyBufferToImage(command_buffer,
                               Buffer,
                               Image,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               CopyRegions.size(),CopyRegions.data()
        );
    }
    
    VkImageMemoryBarrier GetTransferTransitionBarrier(std::uint32_t transfer_id,std::uint32_t src_queue = VK_QUEUE_FAMILY_IGNORED,std::uint32_t dst_queue = VK_QUEUE_FAMILY_IGNORED) const
    {
        return {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext = nullptr,
            
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
            
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            
            .srcQueueFamilyIndex = src_queue,
            .dstQueueFamilyIndex = dst_queue,
            
            .image = Image,
            .subresourceRange = {
                .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel   = 0,
                .levelCount     = 1,
                .baseArrayLayer = CopyRegions[transfer_id].imageSubresource.baseArrayLayer,
                .layerCount     = CopyRegions[transfer_id].imageSubresource.layerCount
            }
        };
    }
    
    VkImageMemoryBarrier GetShaderAccessTransitionBarrier(std::uint32_t transfer_id,std::uint32_t src_queue = VK_QUEUE_FAMILY_IGNORED,std::uint32_t dst_queue = VK_QUEUE_FAMILY_IGNORED) const
    {
        return {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext = nullptr,
            
            .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
            
            .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            
            .srcQueueFamilyIndex = src_queue,
            .dstQueueFamilyIndex = dst_queue,
            
            .image = Image,
            .subresourceRange = {
                .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel   = 0,
                .levelCount     = 1,
                .baseArrayLayer = CopyRegions[transfer_id].imageSubresource.baseArrayLayer,
                .layerCount     = CopyRegions[transfer_id].imageSubresource.layerCount
            }
        };
    }
};

struct lvkBufferToImageTransferRecorder
{
    std::vector<lvkBufferToImageTransferBatch> ImageTransfers;
    
    std::size_t GetBarrierCount() const
    {
        std::size_t BarrierCount = 0;
        
        for(std::size_t i=0;i < ImageTransfers.size();i++)
        {
            BarrierCount += ImageTransfers[i].CopyRegions.size();
        }
        
        return BarrierCount;
    }
    
    void Resize(std::vector<VkImageMemoryBarrier> &image_barriers)
    {
        std::size_t BarrierCount  = GetBarrierCount();
        
        if(image_barriers.capacity() < BarrierCount)
            {image_barriers.reserve(BarrierCount);}
    }
    
    void RecordTransferBarrier(std::vector<VkImageMemoryBarrier> &image_barriers,VkCommandBuffer command_buffer)
    {
        image_barriers.clear();
        for(std::size_t i=0;i < ImageTransfers.size();i++)
        {
            for(std::size_t j=0;j < ImageTransfers[i].CopyRegions.size();j++)
            {
                image_barriers.push_back(ImageTransfers[i].GetTransferTransitionBarrier(j));
            }
        }
        
        vkCmdPipelineBarrier(command_buffer,
                             VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT,
                             0,
                             0,nullptr,
                             0,nullptr,
                             image_barriers.size(),image_barriers.data()
        );
    }
    
    void RecordShaderAccessBarrier(std::vector<VkImageMemoryBarrier> &image_barriers,VkCommandBuffer command_buffer)
    {
        image_barriers.clear();
        for(std::size_t i=0;i < ImageTransfers.size();i++)
        {
            for(std::size_t j=0;j < ImageTransfers[i].CopyRegions.size();j++)
            {
                image_barriers.push_back(ImageTransfers[i].GetShaderAccessTransitionBarrier(j));
            }
        }
        
        vkCmdPipelineBarrier(command_buffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                             0,
                             0,nullptr,
                             0,nullptr,
                             image_barriers.size(),image_barriers.data()
        );
    }
    
    void RecordTransferOps(VkCommandBuffer command_buffer)
    {
        for(std::size_t i=0;i < ImageTransfers.size();i++)
        {
            ImageTransfers[i].RecordTransferOps(command_buffer);
        }
    }
};

struct lvkBufferToBufferTransferBatch
{
    VkBuffer SrcBuffer;
    VkBuffer DstBuffer;
    
    std::vector<VkBufferCopy> CopyRegions;
    
    VkBufferMemoryBarrier GetOwnershipBarrier(std::uint32_t transfer_id,std::uint32_t src_queue = VK_QUEUE_FAMILY_IGNORED,std::uint32_t dst_queue = VK_QUEUE_FAMILY_IGNORED) const
    {
        return {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            .pNext = nullptr,
            
            .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
            
            .srcQueueFamilyIndex = src_queue,
            .dstQueueFamilyIndex = dst_queue,
            
            .buffer = DstBuffer,
            .offset = CopyRegions[transfer_id].dstOffset,
            .size   = CopyRegions[transfer_id].size
        };
    }
    
    void RecordTransferOps(VkCommandBuffer command_buffer)
    {
        vkCmdCopyBuffer(command_buffer,
                        SrcBuffer,
                        DstBuffer,
                        CopyRegions.size(),
                        CopyRegions.data()
       );
    }
};

struct lvkBufferToBufferTransferRecorder
{
    std::vector<lvkBufferToBufferTransferBatch> BufferTransfers;
    
    std::size_t GetBarrierCount() const
    {
        std::size_t BarrierCount = 0;
        
        for(std::size_t i=0;i < BufferTransfers.size();i++)
        {
            BarrierCount += BufferTransfers[i].CopyRegions.size();
        }
        
        return BarrierCount;
    }
    
    void Resize(std::vector<VkBufferMemoryBarrier> &buffer_barriers)
    {
        std::size_t BarrierCount  = GetBarrierCount();
        
        if(buffer_barriers.capacity() < BarrierCount)
            {buffer_barriers.reserve(BarrierCount);}
    }
    
    void RecordShaderAccessBarrier(std::vector<VkBufferMemoryBarrier> &buffer_barriers,VkCommandBuffer command_buffer)
    {
        buffer_barriers.clear();
        for(std::size_t i=0;i < BufferTransfers.size();i++)
        {
            for(std::size_t j=0;j < BufferTransfers[i].CopyRegions.size();j++)
            {
                buffer_barriers.push_back(BufferTransfers[i].GetOwnershipBarrier(j));
            }
        }
        
        vkCmdPipelineBarrier(command_buffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                             0,
                             0,nullptr,
                             buffer_barriers.size(),buffer_barriers.data(),
                             0,nullptr
        );
    }
    
    void RecordTransferOps(VkCommandBuffer command_buffer)
    {
        for(std::size_t i=0;i < BufferTransfers.size();i++)
        {
            BufferTransfers[i].RecordTransferOps(command_buffer);
        }
    }
};

#endif // LVK_TRANSFER_RECORDER_H
