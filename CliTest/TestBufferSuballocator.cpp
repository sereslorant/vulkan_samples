
#include <lvkResourceManager/lrmSuballocator.h>

#include <iostream>

void PrintAllocations(const lrmBufferSuballocator &suballocator)
{
    std::uint64_t AllocIndex = 0;
    for(auto &Alloc : suballocator.GetAllocationList())
    {
        std::cout << "Allocation " << AllocIndex << std::endl;
        std::cout << "Used:  " << Alloc.Used << std::endl;
        std::cout << "Offset: " << Alloc.Offset << std::endl;
        std::cout << "Size:  " << Alloc.Size << std::endl;
        AllocIndex++;
    }
}

int main(int argc, char *argv[])
{
    lrmBufferSuballocator Suballocator(100);
    std::cout << "Creation" << std::endl;
    PrintAllocations(Suballocator);
    
    auto First  = Suballocator.Allocate(10,8);
    std::cout << "Alloc alignment 10 size 8" << std::endl;
    PrintAllocations(Suballocator);
    
    auto Second = Suballocator.Allocate(10,8);
    std::cout << "Alloc alignment 10 size 8" << std::endl;
    PrintAllocations(Suballocator);
    
    auto Third  = Suballocator.Allocate(10,8);
    std::cout << "Alloc alignment 10 size 8" << std::endl;
    PrintAllocations(Suballocator);
    
    First = {};
    std::cout << "Dealloc first" << std::endl;
    PrintAllocations(Suballocator);
    
    Third = {};
    std::cout << "Dealloc third" << std::endl;
    PrintAllocations(Suballocator);
    
    auto Last   = Suballocator.Allocate(10,4);
    std::cout << "Alloc alignment 10 size 4" << std::endl;
    PrintAllocations(Suballocator);
    
    return 0;
}

