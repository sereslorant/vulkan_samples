#ifndef LVK_RESOURCE_TEST_UTILS_H
#define LVK_RESOURCE_TEST_UTILS_H

#include <lvkResourceManager/lvkResourceGroupLoader.h>

struct TestImageGroup : public liImageGroupSrc
{
    constexpr static std::uint32_t WIDTH  = 2;
    constexpr static std::uint32_t HEIGHT = 2;
    constexpr static std::uint32_t IMAGE_COMPONENT_COUNT = 4;
    
    constexpr static std::uint64_t SINGLE_IMAGE_SIZE = WIDTH * HEIGHT * IMAGE_COMPONENT_COUNT * sizeof(float);
    
    constexpr static std::uint32_t IMAGE_COUNT = 4;
    
    std::array<std::array<float,WIDTH*HEIGHT*IMAGE_COMPONENT_COUNT>,IMAGE_COUNT> Images;
    
    virtual void LoadMetadata() override
    {
        std::cout << "Test load image metadata called." << std::endl;
    }
    
    virtual std::uint32_t GetTexelSize() const override
    {
        return IMAGE_COMPONENT_COUNT * sizeof(float);
    }
    
    std::uint32_t GetSingleImageSize() const override
    {
        return SINGLE_IMAGE_SIZE;
    }
    
    std::uint32_t GetImageCount() const override
    {
        return IMAGE_COUNT;
    }
    
    void ReadData(std::uint32_t first_image,std::uint32_t image_count,lMemoryView &dst_region_view) override
    {
        for(int j=0;j < image_count;j++)
        {
            lMemoryView DstView = dst_region_view.GetView(
                j*GetSingleImageSize(),
                GetSingleImageSize()
            );
            
            DstView.MemCpySec(Images[first_image + j].data(),GetSingleImageSize());
        }
    }
    
    virtual void FinishLoading() override
    {
        std::cout << "Test close image file called." << std::endl;
    }
    
    TestImageGroup(const std::array<std::array<float,WIDTH*HEIGHT*IMAGE_COMPONENT_COUNT>,IMAGE_COUNT> &images)
        :Images(images)
    {}
};

struct TestMeshGroup : public liMeshGroupSrc
{
    constexpr static unsigned int NUM_VERTICES = 7;
    constexpr static unsigned int NUM_COMPONENTS = 2;
    std::array<float,NUM_VERTICES*NUM_COMPONENTS> VertexData;
    std::array<float,NUM_VERTICES*NUM_COMPONENTS> TexCoordData;
    
    constexpr static unsigned int NUM_INDICES = 6;
    std::array<std::uint32_t,NUM_INDICES> IndexData;
    
    virtual void LoadMetadata() override
    {
        std::cout << "Test load mesh metadata called." << std::endl;
    }
    
    std::size_t GetVertexDataSize() const override
    {
        return (VertexData.size() + TexCoordData.size()) * sizeof(float);
    }
    
    std::size_t GetIndexDataSize() const override
    {
        return IndexData.size() * sizeof(std::uint32_t);
    }
    
    void ReadVertexOffsets(std::uint32_t &binding_count,std::uint32_t binding_offsets[],const std::uint32_t max_bindings) const override
    {
        binding_count = 0;
        binding_offsets[binding_count] = 0;
        
        binding_count++;
        binding_offsets[binding_count] = VertexData.size() * sizeof(float);
        
        binding_count++;
    }
    
    void LoadVertexData(lMemoryView &vertex_data_region) override
    {
        lMemoryView VertexMemoryView = vertex_data_region.GetView(0,VertexData.size() * sizeof(float));
        VertexMemoryView.MemCpySec(VertexData.data(),VertexData.size() * sizeof(float));
        
        lMemoryView TexCoordMemoryView = vertex_data_region.GetView(VertexData.size() * sizeof(float),TexCoordData.size() * sizeof(float));
        TexCoordMemoryView.MemCpySec(TexCoordData.data(),TexCoordData.size() * sizeof(float));
    }
    
    void LoadIndexData(lMemoryView &index_data_region) override
    {
        index_data_region.MemCpySec(IndexData.data(),GetIndexDataSize());
    }
    
    virtual void FinishLoading(lrMeshGroup &mesh_group) override
    {
        std::cout << "Test close mesh file called." << std::endl;
    }
    
    TestMeshGroup(
        const std::array<float,NUM_VERTICES*NUM_COMPONENTS> &vertex_data,
        const std::array<float,NUM_VERTICES*NUM_COMPONENTS> &tex_coord_data,
        const std::array<std::uint32_t,NUM_INDICES>         &index_data
    )
        :VertexData(vertex_data),
         TexCoordData(tex_coord_data),
         IndexData(index_data)
    {}
};

#endif // LVK_RESOURCE_TEST_UTILS_H
