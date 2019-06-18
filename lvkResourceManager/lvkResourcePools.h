#ifndef LVK_RESOURCE_POOLS_H
#define LVK_RESOURCE_POOLS_H

#include <functional>

#include <lvkUtils/lvkResourceFactory.h>

#include "lrmSuballocator.h"

/*
 * Pools
 */

// Buffers

struct lvkBufferPool
{
    lvkBufferResource     BufferPoolResource;
    lrmBufferSuballocator Suballocator;
    
    lvkBufferPool(lvkBufferResource &&buffer_pool_resource,std::size_t buffer_size)
        :BufferPoolResource(std::move(buffer_pool_resource)),Suballocator(buffer_size)
    {}
};

// Persistent mapped buffer

struct lvkPMappedBufferPool : public lvkBufferPool
{
    lvkMappedMemory MappedMemory;
    
    lvkPMappedBufferPool(lvkBufferResource &&buffer_pool_resource,std::size_t buffer_size)
        :lvkBufferPool(std::move(buffer_pool_resource),buffer_size)
    {
        MappedMemory = BufferPoolResource.Allocation.Map();
    }
};

// Textures

struct lvkImagePool
{
    lvkImageResource     ImagePoolResource;
    lrmImageSuballocator Suballocator;
    
    lvkImagePool(lvkImageResource &&image_pool_resource,std::size_t texture_count)
        :ImagePoolResource(std::move(image_pool_resource)),Suballocator(texture_count)
    {}
};

/*
 * Pool categories
 */

struct lvkBufferPoolCategory
{
    lvkResourceFactory BufferFactory;
    
    const VkDeviceSize       BufferPoolSize;
    const VkBufferUsageFlags UsageFlags;
    
    //std::shared_mutex InstanceLock;
    std::list<lvkBufferPool> Instances;
    
    void CreateInstance()
    {
        //std::unique_lock guard(InstanceLock);
        
        std::cout << "Usage flags: " << UsageFlags << std::endl;
        
        Instances.emplace_back(
            BufferFactory.CreateBuffer(
                BufferPoolSize,
                UsageFlags
            ),
            BufferPoolSize
        );
    }
    
    bool GetFirstInstance(std::function<bool(lvkBufferPool &)> function)
    {
        //std::shared_lock guard(InstanceLock);
        
        bool Successful = false;
        for(auto &BufferPool : Instances)
        {
            Successful = function(BufferPool);
            
            if(Successful)
                {break;}
        }
        
        return Successful;
    }
    
    bool CreateIfFailed(std::function<bool(lvkBufferPool &)> function)
    {
        bool Success = GetFirstInstance(function);
        
        if(!Success)
        {
            CreateInstance();
            
            //std::shared_lock guard(InstanceLock);
            Success = function(Instances.back());
        }
        
        return Success;
    }
    
    lvkBufferPoolCategory(VkDevice device,std::unique_ptr<lvkMemoryAllocProcElement> &&allocator,VkDeviceSize buffer_pool_size,VkBufferUsageFlags usage_flags)
        :BufferFactory(device,std::move(allocator)),BufferPoolSize(buffer_pool_size),UsageFlags(usage_flags)
    {}
};

struct lvkPMappedBufferPoolCategory
{
    lvkResourceFactory BufferFactory;
    
    const VkDeviceSize       BufferPoolSize;
    const VkBufferUsageFlags UsageFlags;
    
    //std::shared_mutex InstanceLock;
    std::list<lvkPMappedBufferPool> Instances;
    
    void CreateInstance()
    {
        //std::unique_lock guard(InstanceLock);
        
        std::cout << "Usage flags: " << UsageFlags << std::endl;
        
        Instances.emplace_back(
            BufferFactory.CreateBuffer(
                BufferPoolSize,
                UsageFlags
            ),
            BufferPoolSize
        );
    }
    
    bool GetFirstInstance(std::function<bool(lvkPMappedBufferPool &)> function)
    {
        //std::shared_lock guard(InstanceLock);
        
        bool Successful = false;
        for(auto &BufferPool : Instances)
        {
            Successful = function(BufferPool);
            
            if(Successful)
                {break;}
        }
        
        return Successful;
    }
    
    bool CreateIfFailed(std::function<bool(lvkPMappedBufferPool &)> function)
    {
        bool Success = GetFirstInstance(function);
        
        if(!Success)
        {
            CreateInstance();
            
            //std::shared_lock guard(InstanceLock);
            Success = function(Instances.back());
        }
        
        return Success;
    }
    
    lvkPMappedBufferPoolCategory(VkDevice device,std::unique_ptr<lvkMemoryAllocProcElement> &&allocator,VkDeviceSize buffer_pool_size,VkBufferUsageFlags usage_flags)
        :BufferFactory(device,std::move(allocator)),BufferPoolSize(buffer_pool_size),UsageFlags(usage_flags)
    {}
};

struct lvkImagePoolCategory
{
    lvkResourceFactory ImageFactory;
    
    const std::uint32_t     TextureWidth;
    const std::uint32_t     TextureHeight;
    const VkFormat          TextureFormat;
    const std::uint32_t     PoolSize;
    const VkImageUsageFlags UsageFlags;
    
    //std::shared_mutex InstanceLock;
    std::list<lvkImagePool> Instances;
    
    void CreateInstance()
    {
        //std::unique_lock guard(InstanceLock);
        
        std::cout << "Usage flags: " << UsageFlags << std::endl;
        
        Instances.emplace_back(
            ImageFactory.CreateImage(
                    TextureWidth,
                    TextureHeight,
                    PoolSize,
                    TextureFormat,
                    UsageFlags
                ),
            PoolSize
        );
    }
    
    bool GetFirstInstance(std::function<bool(lvkImagePool &)> function)
    {
        //std::shared_lock guard(InstanceLock);
        
        bool Successful = false;
        for(auto &BufferPool : Instances)
        {
            Successful = function(BufferPool);
            
            if(Successful)
                {break;}
        }
        
        return Successful;
    }
    
    bool CreateIfFailed(std::function<bool(lvkImagePool &)> function)
    {
        bool Success = GetFirstInstance(function);
        
        if(!Success)
        {
            CreateInstance();
            
            //std::shared_lock guard(InstanceLock);
            Success = function(Instances.back());
        }
        
        return Success;
    }
    
    lvkImagePoolCategory(
        VkDevice device,std::unique_ptr<lvkMemoryAllocProcElement> &&allocator,
        std::uint32_t texture_width,std::uint32_t texture_height,VkFormat texture_format,std::uint32_t pool_size,VkImageUsageFlags usage_flags
    )
        :ImageFactory(device,std::move(allocator)),
         TextureWidth(texture_width),
         TextureHeight(texture_height),
         TextureFormat(texture_format),
         PoolSize(pool_size),
         UsageFlags(usage_flags)
    {}
};

#endif // LVK_RESOURCE_POOLS_H
