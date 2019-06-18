
#include <lvkResourceManager/lrmSuballocator.h>

#include <iostream>

void PrintSuballocator(const lrmImageSuballocator &allocator)
{
    std::cout << "Free image count: " << allocator.GetFreeImageCount() << std::endl;
    for(std::size_t i=0;i < allocator.GetImageIsFreeList().size();i++)
    {
        if(allocator.GetImageIsFreeList()[i])
        {
            std::cout << "Image " << i <<  " is free" << std::endl;
        }
        else
        {
            std::cout << "Image " << i <<  " is not free" << std::endl;
        }
    }
}

void PrintAllocations(const std::vector<std::size_t> &image_list)
{
    std::cout << "Image count: " << image_list.size() << std::endl;
    std::cout << "Allocated images: ";
    for(auto image_id : image_list)
    {
        std::cout << image_id << ";";
    }
    std::cout << std::endl;
}

int main(int argc,char *argv[])
{
    lrmImageSuballocator Suballocator(5);
    PrintSuballocator(Suballocator);
    
    auto First  = Suballocator.Allocate(1);
    PrintSuballocator(Suballocator);
    PrintAllocations(First.GetTextureAllocs());
    auto Second = Suballocator.Allocate(1);
    PrintSuballocator(Suballocator);
    PrintAllocations(Second.GetTextureAllocs());
    auto Third  = Suballocator.Allocate(1);
    PrintSuballocator(Suballocator);
    PrintAllocations(Third.GetTextureAllocs());
    
    //Suballocator.Deallocate(Second);
    Second = {};
    PrintSuballocator(Suballocator);
    PrintAllocations(Second.GetTextureAllocs());
    
    auto Last = Suballocator.Allocate(3);
    PrintSuballocator(Suballocator);
    PrintAllocations(Last.GetTextureAllocs());
    
    return 0;
}
