#ifndef LVK_MEMORY_UTILS_H
#define LVK_MEMORY_UTILS_H

#include <vulkan/vulkan.h>

#include <cstring>

//TMP!!!!!!!!!!!!!!!!
#include <iostream>
//TMP!!!!!!!!!!!!!!!!

std::size_t lPaddedStructSize(std::size_t min_offset_alignment,std::size_t struct_size)
{
    std::size_t StructDiv = struct_size/min_offset_alignment;
    
    if((struct_size % min_offset_alignment) != 0)
    {
        StructDiv += 1;
    }
    
    return StructDiv * min_offset_alignment;
}

class lBlockMemoryView
{
private:
    char *Begin;
    std::size_t ItemCount;
    std::size_t StructSize;
    
public:
    
    char *GetBegin()
    {
        return Begin;
    }
    
    char *GetEnd()
    {
        return Begin + ItemCount * StructSize;
    }
    
    char *GetBlock(std::size_t index)
    {
        if(index < ItemCount)
        {
            return Begin + index*StructSize;
        }
        else
        {
            return nullptr;
        }
    }
    
    lBlockMemoryView(char *begin,std::size_t item_count,std::size_t struct_size)
        :Begin(begin),ItemCount(item_count),StructSize(struct_size)
    {}
};

class lMemoryView
{
private:
    std::size_t Size;
    char *Ptr;

public:

    std::size_t GetSize()
    {
        return Size;
    }

    void *GetPtr()
    {
        return Ptr;
    }

    void *GetEndPtr()
    {
        return Ptr + Size;
    }

    bool MemCpySec(void *src,std::size_t src_size)
    {
        if(src_size <= Size)
        {
            std::memcpy(Ptr,src,src_size);
            return true;
        }
        
        return false;
    }

    lMemoryView GetView(std::size_t offset,std::size_t size)
    {
        lMemoryView Result(Ptr + offset,size);
        
        if(Result.GetPtr() >= GetPtr() && Result.GetPtr() <= GetEndPtr() && Result.GetEndPtr() <= GetEndPtr())
        {
            return Result;
        }
        
        return lMemoryView();
    }

    lMemoryView(char *ptr = nullptr,std::size_t size = 0)
        :Size(size),Ptr(ptr)
    {}
};

class lvkMappedMemory
{
private:
    std::size_t Size = 0;
    void *Ptr = nullptr;
    
    VkDevice Device;
    VkDeviceMemory Memory;
    
public:
    
    bool IsSuccessful()
    {
        return Ptr != nullptr;
    }
    
    lMemoryView GetView()
    {
        return lMemoryView(static_cast<char *>(Ptr),Size);
    }
    
    void operator=(lvkMappedMemory &&other)
    {
        Size = other.Size;
        Ptr  = other.Ptr;
        
        Device = other.Device;
        Memory = other.Memory;
        
        other.Device = VK_NULL_HANDLE;
        other.Memory = VK_NULL_HANDLE;
    }
    
    lvkMappedMemory()
        :Device(VK_NULL_HANDLE),Memory(VK_NULL_HANDLE)
    {}
    
    lvkMappedMemory(VkDevice device,VkDeviceMemory memory,VkDeviceSize offset,VkDeviceSize size)
        :Device(device),Memory(memory)
    {
        void* TmpPtr;
        if(vkMapMemory(Device,Memory,offset,size,0,&TmpPtr) == VK_SUCCESS)
        {
            Size = size;
            Ptr = TmpPtr;
            
            std::cout << "Memory mapped" << std::endl;
        }
    }
    
    lvkMappedMemory(lvkMappedMemory &&other)
    {
        Size = other.Size;
        Ptr  = other.Ptr;
        
        Device = other.Device;
        Memory = other.Memory;
        
        other.Device = VK_NULL_HANDLE;
        other.Memory = VK_NULL_HANDLE;
    }
    
    ~lvkMappedMemory()
    {
        if(Device != VK_NULL_HANDLE)
        {
            vkUnmapMemory(Device,Memory);
            
            std::cout << "Memory unmapped" << std::endl;
        }
    }
};

class lvkMemoryAllocation
{
private:
    VkDevice       Device      = VK_NULL_HANDLE;
    VkDeviceMemory Memory      = VK_NULL_HANDLE;
    VkDeviceSize   Size        = 0;
    bool           HostVisible = false;
    bool           Resident    = false;
    
public:
    
    VkDeviceMemory GetMemory()
    {
        return Memory;
    }
    
    VkDeviceSize GetSize()
    {
        return Size;
    }
    
    bool IsHostVisible() const
    {
        return HostVisible;
    }
    
    bool IsResident() const
    {
        return Resident;
    }
    
    lvkMappedMemory MapRange(VkDeviceSize offset,VkDeviceSize size)
    {
        return {Device,Memory,offset,size};
    }
    
    lvkMappedMemory Map()
    {
        return MapRange(0,Size);
    }
    
    void operator=(lvkMemoryAllocation &&other)
    {
        if(Device != VK_NULL_HANDLE)
        {
            vkFreeMemory(Device,Memory,nullptr);
            
            std::cout << "Deleting memory" << std::endl;
        }
        
        Device      = other.Device;
        Memory      = other.Memory;
        Size        = other.Size;
        HostVisible = other.HostVisible;
        Resident    = other.Resident;
        
        other.Device = VK_NULL_HANDLE;
        other.Memory = VK_NULL_HANDLE;
    }
    
    lvkMemoryAllocation() = default;
    
    lvkMemoryAllocation(VkDevice device,VkDeviceMemory memory,VkDeviceSize size,bool host_visible,bool resident)
        :Device(device),Memory(memory),Size(size),HostVisible(host_visible),Resident(resident)
    {}
    
    lvkMemoryAllocation(lvkMemoryAllocation &&other)
    {
        Device      = other.Device;
        Memory      = other.Memory;
        Size        = other.Size;
        HostVisible = other.HostVisible;
        Resident    = other.Resident;
        
        other.Device = VK_NULL_HANDLE;
        other.Memory = VK_NULL_HANDLE;
    }
    
    ~lvkMemoryAllocation()
    {
        if(Device != VK_NULL_HANDLE)
        {
            vkFreeMemory(Device,Memory,nullptr);
            
            std::cout << "Deleting memory" << std::endl;
        }
    }
};

constexpr std::uint32_t LVK_STATIC_MEMORY_PROPERTIES   = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

constexpr std::uint32_t LVK_FALLBACK_MEMORY_PROPERTIES = 0x0;
    
constexpr std::uint32_t LVK_TRANSFER_MEMORY_PROPERTIES = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

constexpr std::uint32_t LVK_STREAM_MEMORY_PROPERTIES   = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
                                                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
                                                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

#include <memory>

class lvkMemoryAllocProcElement
{
private:
    VkDevice Device;
    const VkPhysicalDeviceMemoryProperties &MemoryProperties;
    
    std::uint32_t PropertyFlags;
    
    std::unique_ptr<lvkMemoryAllocProcElement> Successor = nullptr;
    
public:
    
    lvkMemoryAllocation RequestAllocation(VkDeviceSize allocation_size,std::uint32_t memory_type_bits)
    {
        VkMemoryAllocateInfo AllocateInfo = {};
        AllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        AllocateInfo.allocationSize = allocation_size;
        
        std::cout << "Allocator prop flags: " << PropertyFlags    << std::endl;
        std::cout << "Allocator mem types:  " << memory_type_bits << std::endl;
        
        bool FoundType = false;
        for(std::uint32_t i=0;i < MemoryProperties.memoryTypeCount;i++)
        {
            if(((1 << i) & memory_type_bits) && (MemoryProperties.memoryTypes[i].propertyFlags & PropertyFlags))
            {
                AllocateInfo.memoryTypeIndex = i;
                FoundType = true;
                break;
            }
        }
        
        VkDeviceMemory Memory = VK_NULL_HANDLE;
        if(!FoundType || (vkAllocateMemory(Device,&AllocateInfo,nullptr,&Memory) != VK_SUCCESS))
        {
            if(Successor != nullptr)
                {return Successor->RequestAllocation(allocation_size,memory_type_bits);}
            else
                {return {};}
        }
        
        bool IsHostVisible = MemoryProperties.memoryTypes[AllocateInfo.memoryTypeIndex].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        bool IsResident    = MemoryProperties.memoryTypes[AllocateInfo.memoryTypeIndex].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        
        std::cout << "Allocation successful." << std::endl;
        if(IsHostVisible)
            {std::cout << "Host visible." << std::endl;}
        if(IsResident)
            {std::cout << "Is resident." << std::endl;}
        
        return {Device,Memory,allocation_size,IsHostVisible,IsResident};
    }
    
    void SetSuccessor(std::unique_ptr<lvkMemoryAllocProcElement> &&successor)
    {
        Successor = std::move(successor);
    }
    
    lvkMemoryAllocProcElement(VkDevice device,const VkPhysicalDeviceMemoryProperties &memory_properties,std::uint32_t property_flags)
        :Device(device),MemoryProperties(memory_properties),PropertyFlags(property_flags)
    {}
};

class lvkMemoryUtility
{
private:
    VkDevice Device;
    VkPhysicalDeviceMemoryProperties MemoryProperties;
    
public:
    
    std::unique_ptr<lvkMemoryAllocProcElement> CreateAllocator(std::uint32_t property_flags)
    {
        return std::unique_ptr<lvkMemoryAllocProcElement>(
            new lvkMemoryAllocProcElement(Device,MemoryProperties,property_flags)
        );
    }
    
    lvkMemoryUtility(VkDevice device = VK_NULL_HANDLE,VkPhysicalDevice physical_device = VK_NULL_HANDLE)
        :Device(device)
    {
        if(physical_device != VK_NULL_HANDLE)
        {
            vkGetPhysicalDeviceMemoryProperties(physical_device,&MemoryProperties);
        }
    }
};

#endif // LVK_MEMORY_UTILS_H
