#ifndef LVK_DESCRIPTOR_SET_UTILS_H
#define LVK_DESCRIPTOR_SET_UTILS_H

#include <lvkUtils/lvkSmartPtrs/lvkDescriptorPool.h>

struct lvkDescriptorPoolSizes
{
    std::size_t          PoolSizeCount = 0;
    VkDescriptorPoolSize DescriptorPoolSizes[VK_DESCRIPTOR_TYPE_RANGE_SIZE] = {};
    
    void SetDescriptorPoolSize(VkDescriptorType descriptor_type,std::uint32_t descriptor_count)
    {
        for(std::uint32_t i=0;i < PoolSizeCount;i++)
        {
            if(DescriptorPoolSizes[i].type == descriptor_type)
            {
                DescriptorPoolSizes[i].descriptorCount = descriptor_count;
                return;
            }
        }
        
        DescriptorPoolSizes[PoolSizeCount].type            = descriptor_type;
        DescriptorPoolSizes[PoolSizeCount].descriptorCount = descriptor_count;
        PoolSizeCount++;
    }
    
    void AssignToDescriptorPoolCreateInfo(VkDescriptorPoolCreateInfo &descriptor_pool_create_info)
    {
        if(PoolSizeCount > 0)
        {
            descriptor_pool_create_info.poolSizeCount = PoolSizeCount;
            descriptor_pool_create_info.pPoolSizes    = DescriptorPoolSizes;
        }
    }
};

class lvkTextureDescriptorWriteBatch
{
private:
    VkDevice Device;
    std::vector<VkDescriptorImageInfo> ImageInfos;
    std::vector<VkWriteDescriptorSet>  DescriptorWrites;
    
public:
    
    std::size_t GetWriteCount()
    {
        return DescriptorWrites.size();
    }
    
    void SetDescriptorWrite(std::size_t id,VkImageView image_view,VkDescriptorSet descriptor_set)
    {
        ImageInfos[id] = {};
        ImageInfos[id].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        ImageInfos[id].imageView   = image_view;
        
        DescriptorWrites[id] = {};
        
        DescriptorWrites[id].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        DescriptorWrites[id].dstSet          = descriptor_set;
        DescriptorWrites[id].dstBinding      = 1;
        DescriptorWrites[id].dstArrayElement = 0;
        
        DescriptorWrites[id].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        DescriptorWrites[id].descriptorCount = 1;
        
        DescriptorWrites[id].pImageInfo = &ImageInfos[id];
    }
    
    lvkTextureDescriptorWriteBatch(VkDevice device,std::size_t write_count)
        :Device(device),ImageInfos(write_count),DescriptorWrites(write_count)
    {}
    
    ~lvkTextureDescriptorWriteBatch()
    {
        vkUpdateDescriptorSets(Device,DescriptorWrites.size(),DescriptorWrites.data(),0,nullptr);
    }
};

#include <vector>

class lvkDescriptorPoolUtil
{
private:
    VkDevice         Device;
    VkDescriptorPool DescriptorPool;
    
public:
    
    VkDescriptorSet Allocate(VkDescriptorSetLayout descriptor_set_layout)
    {
        VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo = {};
        DescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        DescriptorSetAllocateInfo.descriptorPool     = DescriptorPool;
        DescriptorSetAllocateInfo.descriptorSetCount = 1;
        DescriptorSetAllocateInfo.pSetLayouts        = &descriptor_set_layout;
        
        VkDescriptorSet Result;
        if(vkAllocateDescriptorSets(Device,&DescriptorSetAllocateInfo,&Result) != VK_SUCCESS)
        {
            std::cout << "Error while creating descriptor set" << std::endl;
            return VK_NULL_HANDLE;
        }
        return Result;
    }
    
    std::vector<VkDescriptorSet> Allocate(const std::vector<VkDescriptorSetLayout> &descriptor_set_layouts)
    {
        VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo = {};
        DescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        DescriptorSetAllocateInfo.descriptorPool     = DescriptorPool;
        DescriptorSetAllocateInfo.descriptorSetCount = descriptor_set_layouts.size();
        DescriptorSetAllocateInfo.pSetLayouts        = descriptor_set_layouts.data();
        
        std::vector<VkDescriptorSet> Result(descriptor_set_layouts.size());
        if(vkAllocateDescriptorSets(Device,&DescriptorSetAllocateInfo,Result.data()) != VK_SUCCESS)
        {
            std::cout << "Error while creating descriptor set" << std::endl;
            return {};
        }
        return Result;
    }
    
    void Reset()
    {
        vkResetDescriptorPool(Device,DescriptorPool,0x0);
    }
    
    void operator=(const lvkDescriptorPoolUtil &other)
    {
        Device         = other.Device;
        DescriptorPool = other.DescriptorPool;
    }
    
    lvkDescriptorPoolUtil(VkDevice device = VK_NULL_HANDLE,VkDescriptorPool descriptor_pool = VK_NULL_HANDLE)
        :Device(device),DescriptorPool(descriptor_pool)
    {}
};

#endif // LVK_DESCRIPTOR_SET_UTILS_H
