#ifndef LVK_RESOURCE_FACTORY_H
#define LVK_RESOURCE_FACTORY_H

#include <lvkUtils/lvkMemoryUtils.h>

#include <lvkUtils/lvkSmartPtrs/lvkBuffer.h>
#include <lvkUtils/lvkSmartPtrs/lvkImage.h>

#include <lvkUtils/lvkSmartPtrs/lvkImageView.h>

#include <lvkUtils/lvkDefaultCreateInfos.h>

/*
 * Resource factory
 */

struct lvkBufferResource
{
    lvkBuffer           Buffer;
    lvkMemoryAllocation Allocation;
};

struct lvkImageResource
{
    lvkImage            Image;
    lvkMemoryAllocation Allocation;
    
    std::uint32_t Width;
    std::uint32_t Height;
    
    std::uint32_t ImageCount;
    
    lvkImageView CreateImageView(VkDevice device,VkFormat format)
    {
        VkImageViewCreateInfo ImageViewCreateInfo = LVK_2D_ARRAY_IMG_VIEW_CREATE_INFO;
        
        ImageViewCreateInfo.image                       = Image.get();
        ImageViewCreateInfo.format                      = format;
        ImageViewCreateInfo.subresourceRange.layerCount = ImageCount;
        
        std::cout << "Creating image view" << std::endl;
        VkImageView ImageView;
        if(vkCreateImageView(device,&ImageViewCreateInfo,nullptr,&ImageView) != VK_SUCCESS)
        {
            std::cout << "Couldn't create image view " << std::endl;
        }
        
        return lvkImageView{ImageView,{device}};
    }
};

class lvkResourceFactory
{
private:
    VkDevice Device;
    std::unique_ptr<lvkMemoryAllocProcElement> Allocator;
    
public:
    
    lvkBufferResource CreateBuffer(VkDeviceSize size,VkBufferUsageFlags usage)
    {
        VkBufferCreateInfo BufferCreateInfo = LVK_EMPTY_BUFFER_CREATE_INFO;
        BufferCreateInfo.size  = size;
        BufferCreateInfo.usage = usage;
        
        std::cout << "Creating buffer" << std::endl;
        VkBuffer Buffer = VK_NULL_HANDLE;
        if(vkCreateBuffer(Device,&BufferCreateInfo,nullptr,&Buffer) != VK_SUCCESS)
        {
            std::cerr << "Error while creating buffer" << std::endl;
        }
        
        VkMemoryRequirements MemoryRequirements;
        vkGetBufferMemoryRequirements(Device,Buffer,&MemoryRequirements);
        
        lvkMemoryAllocation Allocation = Allocator->RequestAllocation(MemoryRequirements.size,MemoryRequirements.memoryTypeBits);
        
        vkBindBufferMemory(Device,Buffer,Allocation.GetMemory(),0);
        
        return {
            .Buffer = lvkBuffer(Buffer,{Device}),
            .Allocation = std::move(Allocation),
        };
    }
    
    lvkImageResource CreateImage(std::uint32_t width,std::uint32_t height,std::uint32_t image_count,VkFormat format,VkImageUsageFlags usage)
    {
            VkImageCreateInfo ImageCreateInfo = LVK_EMPTY_2D_IMAGE_CREATE_INFO;
            
            ImageCreateInfo.usage = usage;
            
            ImageCreateInfo.extent.width  = width;
            ImageCreateInfo.extent.height = height;
            ImageCreateInfo.arrayLayers   = image_count;
            
            ImageCreateInfo.format = format;
            
            std::cout << "Creating image" << std::endl;
            VkImage Image = VK_NULL_HANDLE;
            if(vkCreateImage(Device,&ImageCreateInfo,nullptr,&Image) != VK_SUCCESS)
            {
                std::cout << "Error while creating image" << std::endl;
            }
            
            VkMemoryRequirements MemoryRequirements;
            vkGetImageMemoryRequirements(Device,Image,&MemoryRequirements);
            
            lvkMemoryAllocation Allocation = Allocator->RequestAllocation(MemoryRequirements.size,MemoryRequirements.memoryTypeBits);
            
            vkBindImageMemory(Device,Image,Allocation.GetMemory(),0);
            
            return {
                .Image = lvkImage{Image,{Device}},
                .Allocation = std::move(Allocation),
                
                .Width = width,
                .Height = height,
                
                .ImageCount = image_count
            };
    }
    
    lvkResourceFactory(VkDevice device,std::unique_ptr<lvkMemoryAllocProcElement> &&allocator)
        :Device(device),Allocator(std::move(allocator))
    {}
};

#endif // LVK_RESOURCE_FACTORY_H
