#ifndef LRM_SUBALLOCATOR_H
#define LRM_SUBALLOCATOR_H

/*
 * Buffer suballocator
 */

#include <cinttypes>
#include <list>

#include <mutex>

struct lrmSubBufferMetadata
{
    bool Used = false;
    std::uint64_t Offset = 0;
    std::uint64_t Size  = 0;
};

struct lrmBufferAllocList
{
    using lrmAllocationList = std::list<lrmSubBufferMetadata>;
    using lrmAllocIterator  = lrmAllocationList::iterator;
    
    lrmAllocationList Allocations;
    std::mutex Lock;
    
    lrmAllocIterator InsertAlloc(const lrmAllocIterator &position,bool used,std::uint64_t size)
    {
        lrmSubBufferMetadata &Metadata = *position;
        
        lrmSubBufferMetadata NewAlloc;
        NewAlloc.Used  = used;
        NewAlloc.Offset = Metadata.Offset;
        NewAlloc.Size  = size;

        Metadata.Offset += size;
        Metadata.Size  -= size;
        Allocations.insert(position,NewAlloc);
        
        return std::prev(position);
    }
    
    void MergeWithPrevious(const lrmAllocIterator &current)
    {
        if(current != Allocations.begin())
        {
            auto Previous = std::prev(current);
            if(!Previous->Used)
            {
                current->Offset -= Previous->Size;
                current->Size  += Previous->Size;
                Allocations.erase(Previous);
            }
        }
    }
    
    void MergeWithNext(const lrmAllocIterator &current)
    {
        if(current != Allocations.end())
        {
            auto Next = std::next(current);
            if(!Next->Used)
            {
                current->Size += Next->Size;
                Allocations.erase(Next);
            }
        }
    }
    
    lrmAllocIterator Allocate(std::uint64_t alignment,std::uint64_t size)
    {
        std::lock_guard<std::mutex> guard(Lock);
        
        auto Current = Allocations.begin();
        while(Current != Allocations.end())
        {
            lrmSubBufferMetadata &Metadata = *Current;
            if(!Metadata.Used && Metadata.Size >= size)
            {
                std::uint64_t BeginThing = Metadata.Offset % alignment;
                
                if(BeginThing != 0 && (Metadata.Size - (alignment - BeginThing)) >= size)
                {
                    InsertAlloc(Current,false,alignment - BeginThing);
                    return InsertAlloc(Current,true,size);
                }
                else
                {
                    return InsertAlloc(Current,true,size);
                }
            }
            
            Current++;
        }
        
        return Allocations.end();
    }
    
    void Deallocate(const lrmAllocIterator &alloc)
    {
        std::lock_guard<std::mutex> guard(Lock);
        
        MergeWithPrevious(alloc);
        MergeWithNext(alloc);
        alloc->Used = false;
    }
    
    lrmBufferAllocList(std::uint64_t p_size)
    {
        lrmSubBufferMetadata FreeSpace;
        FreeSpace.Size = p_size;
        
        Allocations.push_back(FreeSpace);
    }
};

class lrmBufferSuballocGuard
{
private:
    lrmBufferAllocList *Suballocator = nullptr;
    lrmBufferAllocList::lrmAllocIterator SuballocIterator;
    
public:
    
    bool IsValid() const
    {
        return Suballocator != nullptr;
    }
    
    std::uint64_t GetOffset() const
    {
        return SuballocIterator->Offset;
    }
    
    std::uint64_t GetSize() const
    {
        return SuballocIterator->Size;
    }
    
    void operator=(lrmBufferSuballocGuard &&other)
    {
        if(Suballocator != nullptr)
        {
            Suballocator->Deallocate(SuballocIterator);
        }
        
        Suballocator      = other.Suballocator;
        SuballocIterator  = other.SuballocIterator;
        
        other.Suballocator = nullptr;
    }
    
    lrmBufferSuballocGuard()
    {}
    
    lrmBufferSuballocGuard(lrmBufferAllocList *suballocator,lrmBufferAllocList::lrmAllocIterator suballoc_iterator)
        :Suballocator(suballocator),SuballocIterator(suballoc_iterator)
    {}
    
    lrmBufferSuballocGuard(lrmBufferSuballocGuard &&other)
    {
        if(Suballocator != nullptr)
        {
            Suballocator->Deallocate(SuballocIterator);
        }
        
        Suballocator      = other.Suballocator;
        SuballocIterator  = other.SuballocIterator;
        
        other.Suballocator = nullptr;
    }
    
    ~lrmBufferSuballocGuard()
    {
        if(Suballocator != nullptr)
        {
            Suballocator->Deallocate(SuballocIterator);
        }
    }
};

class lrmBufferSuballocator
{
private:
    lrmBufferAllocList AllocationList;
    
public:
    
    /*
     * WARNING For test cases only.
     */
    
    const lrmBufferAllocList::lrmAllocationList &GetAllocationList() const
    {
        return AllocationList.Allocations;
    }
    
    lrmBufferSuballocGuard Allocate(std::uint64_t alignment,std::uint64_t size)
    {
        auto Allocation = AllocationList.Allocate(alignment,size);
        if(Allocation != AllocationList.Allocations.end())
        {
            return {&AllocationList,Allocation};
        }
        
        return {};
    }
    
    lrmBufferSuballocator(std::uint64_t p_size)
        :AllocationList(p_size)
    {}
};

/*
 * Image suballocator
 */

#include <vector>

struct lrmImageAllocationStatus
{
    std::size_t       FreeImageCount;
    std::vector<bool> ImageIsFree;
    
    std::vector<std::size_t> Allocate(std::size_t requested_img_count)
    {
        std::vector<std::size_t> Result;
        
        if(requested_img_count <= FreeImageCount)
        {
            Result.resize(requested_img_count);
            
            std::size_t j = 0;
            for(std::size_t i=0;i < Result.size();i++)
            {
                for(;j<ImageIsFree.size();j++)
                {
                    if(ImageIsFree[j])
                    {
                        ImageIsFree[j] = false;
                        Result[i] = j;
                        break;
                    }
                }
            }
            
            FreeImageCount -= Result.size();
        }
        
        return Result;
    }
    
    void Deallocate(std::vector<std::size_t> &image_list)
    {
        for(std::size_t i=0;i < image_list.size();i++)
        {
            ImageIsFree[image_list[i]] = true;
        }
        
        FreeImageCount += image_list.size();
        
        image_list = {};
    }
    
    lrmImageAllocationStatus(std::size_t image_count)
        :FreeImageCount(image_count),ImageIsFree(image_count,true)
    {}
};

class lrmImageSuballocGuard
{
private:
    lrmImageAllocationStatus *Suballocator = nullptr;
    std::vector<std::size_t> TextureAllocs;
    
public:
    
    std::vector<std::size_t> &GetTextureAllocs()
    {
        return TextureAllocs;
    }
    
    bool IsValid() const
    {
        return Suballocator != nullptr;
    }
    
    void operator=(lrmImageSuballocGuard &&other)
    {
        if(Suballocator != nullptr)
        {
            Suballocator->Deallocate(TextureAllocs);
        }
        
        Suballocator  = other.Suballocator;
        TextureAllocs = other.TextureAllocs;
        
        other.Suballocator = nullptr;
    }
    
    lrmImageSuballocGuard()
    {}
    
    lrmImageSuballocGuard(lrmImageAllocationStatus *suballocator,std::vector<std::size_t> &&texture_allocs)
        :Suballocator(suballocator),TextureAllocs(texture_allocs)
    {}
    
    lrmImageSuballocGuard(lrmImageSuballocGuard &&other)
    {
        if(Suballocator != nullptr)
        {
            Suballocator->Deallocate(TextureAllocs);
        }
        
        Suballocator  = other.Suballocator;
        TextureAllocs = other.TextureAllocs;
        
        other.Suballocator = nullptr;
    }
    
    ~lrmImageSuballocGuard()
    {
        if(Suballocator != nullptr)
        {
            Suballocator->Deallocate(TextureAllocs);
        }
    }
};

class lrmImageSuballocator
{
private:
    lrmImageAllocationStatus AllocationList;
    
public:
    
    /*
     * WARNING For test cases only.
     */
    
    std::size_t GetFreeImageCount() const
    {
        return AllocationList.FreeImageCount;
    }
    
    /*
     * WARNING For test cases only.
     */
    const std::vector<bool> &GetImageIsFreeList() const
    {
        return AllocationList.ImageIsFree;
    }
    
    lrmImageSuballocGuard Allocate(std::uint64_t requested_img_count)
    {
        if(AllocationList.FreeImageCount < requested_img_count)
            {return {};}
        
        return {&AllocationList,AllocationList.Allocate(requested_img_count)};
    }
    
    lrmImageSuballocator(std::uint64_t p_size)
        :AllocationList(p_size)
    {}
};

#endif // LRM_SUBALLOCATOR_H
