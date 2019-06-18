#ifndef TEST_MESH_UTILS_H
#define TEST_MESH_UTILS_H

#include <lvkResourceManager/lrmIstreamMeshGroupLoader.h>

#include <memory>
#include <iostream>


struct lrVertexDataFormat
{
    std::uint32_t VertexOffset   = 0;
    std::uint32_t NormalOffset   = 0;
    std::uint32_t TexCoordOffset = 0;
    std::uint32_t VertexStride   = 0;
    std::uint32_t NormalStride   = 0;
    std::uint32_t TexCoordStride = 0;
    
    void FillVertexDataFormat(std::uint32_t storage,std::uint32_t vertex_count)
    {
        switch(storage)
        {
            case lrmMeshMetadata::INTERLEAVED:
                VertexOffset   = 0;
                NormalOffset   = VertexOffset + lrmMeshMetadata::VERTEX_COMPONENT_COUNT;
                TexCoordOffset = NormalOffset + lrmMeshMetadata::NORMAL_COMPONENT_COUNT;
                
                VertexStride   = lrmMeshMetadata::VERTEX_COMPONENT_COUNT + lrmMeshMetadata::NORMAL_COMPONENT_COUNT + lrmMeshMetadata::TEXCOORD_COMPONENT_COUNT;
                NormalStride   = lrmMeshMetadata::VERTEX_COMPONENT_COUNT + lrmMeshMetadata::NORMAL_COMPONENT_COUNT + lrmMeshMetadata::TEXCOORD_COMPONENT_COUNT;
                TexCoordStride = lrmMeshMetadata::VERTEX_COMPONENT_COUNT + lrmMeshMetadata::NORMAL_COMPONENT_COUNT + lrmMeshMetadata::TEXCOORD_COMPONENT_COUNT;
                break;
                
            case lrmMeshMetadata::DEINTERLEAVED:
                VertexOffset   = 0;
                NormalOffset   = vertex_count * lrmMeshMetadata::VERTEX_COMPONENT_COUNT;
                TexCoordOffset = vertex_count * (lrmMeshMetadata::VERTEX_COMPONENT_COUNT + lrmMeshMetadata::NORMAL_COMPONENT_COUNT);
                
                VertexStride   = lrmMeshMetadata::VERTEX_COMPONENT_COUNT;
                NormalStride   = lrmMeshMetadata::NORMAL_COMPONENT_COUNT;
                TexCoordStride = lrmMeshMetadata::TEXCOORD_COMPONENT_COUNT;
                break;
                
            default:
                break;
        }
    }
};

struct DummyMeshGroupAllocator : public liAttribArrayAllocator
{
    std::unique_ptr<float []>         VertexData;
    std::unique_ptr<std::uint32_t []> IndexData;
    
    float         *CurrentVertexPtr;
    std::uint32_t *CurrentIndexPtr;
    
    void AllocateMemory(std::uint32_t storage,const lMeshGroupSize &mesh_group_size)
    {
        std::int32_t TotalVertexDataSize = 0;
        std::int32_t TotalIndexDataSize = 0;
        
        switch(storage)
        {
            case lrmMeshMetadata::INTERLEAVED:
                TotalVertexDataSize = mesh_group_size.Interleaved.VertexDataSize;
                
                TotalIndexDataSize  = mesh_group_size.Interleaved.IndexDataSize;
                break;
                
            case lrmMeshMetadata::DEINTERLEAVED:
                TotalVertexDataSize += mesh_group_size.Deinterleaved.VertexDataSize;
                TotalVertexDataSize += mesh_group_size.Deinterleaved.NormalDataSize;
                TotalVertexDataSize += mesh_group_size.Deinterleaved.TexCoordDataSize;
                
                TotalIndexDataSize  =  mesh_group_size.Deinterleaved.IndexDataSize;
                break;
        }
        
        VertexData = std::unique_ptr<float []>(new float[TotalVertexDataSize]);
        IndexData  = std::unique_ptr<std::uint32_t []>(new std::uint32_t[TotalIndexDataSize]);
        
        CurrentVertexPtr = VertexData.get();
        CurrentIndexPtr  = IndexData.get();
    }
    
    virtual lVertexDataPtrs RequestAttribArray(const lrmMeshMetadata &mesh_metadata) override
    {
        lVertexDataPtrs Result = {};
        
        switch(mesh_metadata.Storage)
        {
            case lrmMeshMetadata::INTERLEAVED:
                Result.Interleaved = CurrentVertexPtr;
                CurrentVertexPtr += mesh_metadata.GetTotalVertexDataSize();
                break;
                
            case lrmMeshMetadata::DEINTERLEAVED:
                Result.Deinterleaved.Vertices  = CurrentVertexPtr;
                CurrentVertexPtr += mesh_metadata.GetVertexSize();
                Result.Deinterleaved.Normals   = CurrentVertexPtr;
                CurrentVertexPtr += mesh_metadata.GetNormalSize();
                Result.Deinterleaved.TexCoords = CurrentVertexPtr;
                CurrentVertexPtr += mesh_metadata.GetTexCoordSize();
                break;
        }
        
        return Result;
    }
    
    virtual std::uint32_t* RequestIndexArray(const lrmMeshMetadata &mesh_metadata) override
    {
        std::uint32_t *NextIndexData = CurrentIndexPtr;
        
        CurrentIndexPtr += mesh_metadata.GetTotalIndexDataSize();
        
        return NextIndexData;
    }
    
    virtual ~DummyMeshGroupAllocator() // override
    {}
};

void PrintMetadata(const lrmMeshMetadata &mesh_metadata)
{
    switch(mesh_metadata.Storage)
    {
        case lrmMeshMetadata::INTERLEAVED:
            std::cout << "Interleaved" << std::endl;
            break;
        case lrmMeshMetadata::DEINTERLEAVED:
            std::cout << "Deinterleaved" << std::endl;
            break;
        default:
            std::cout << "Error: couldn't load file metadata" << std::endl;
            break;
    }
    
    std::cout << "VertexDataCount: " << mesh_metadata.VertexCount << std::endl;
    std::cout << "MaterialGroupCount: " << mesh_metadata.MaterialGroupCount << std::endl;
    
    for(std::uint32_t i=0;i < mesh_metadata.MaterialGroupCount;i++)
    {
        std::cout << "IndexData " << i << " count: " << mesh_metadata.IndexDataCount[i] << std::endl;
    }
}

void PrintVectors(float array[],std::uint32_t offset,std::uint32_t components,std::uint32_t count,std::uint32_t stride)
{
    float *Begin = &array[offset];
    
    for(int i=0;i < count;i++)
    {
        std::cout << "[";
        for(int j = 0;j < components;j++)
        {
            std::cout << Begin[i*stride + j];
            std::cout << ",";
        }
        std::cout << "]," << std::endl;
    }
}

void PrintVertexData(const lrmMeshMetadata &mesh_metadata,lrVertexDataFormat vertex_data_format,float vertex_data[])
{
    std::cout << "VertexData:" << std::endl;
    PrintVectors(
        vertex_data,
        vertex_data_format.VertexOffset,
        lrmMeshMetadata::VERTEX_COMPONENT_COUNT,
        mesh_metadata.VertexCount,
        vertex_data_format.VertexStride
    );
    
    std::cout << "NormalData:" << std::endl;
    PrintVectors(
        vertex_data,
        vertex_data_format.NormalOffset,
        lrmMeshMetadata::NORMAL_COMPONENT_COUNT,
        mesh_metadata.VertexCount,
        vertex_data_format.NormalStride
    );
    
    std::cout << "TexCoordData:" << std::endl;
    PrintVectors(
        vertex_data,
        vertex_data_format.TexCoordOffset,
        lrmMeshMetadata::TEXCOORD_COMPONENT_COUNT,
        mesh_metadata.VertexCount,
        vertex_data_format.TexCoordStride
    );
}

void PrintIndexData(const lrmMeshMetadata &mesh_metadata,lrMeshGroup::lrIndexedSubMesh material_groups[],std::uint32_t index_data[])
{
    for(std::uint32_t i=0;i < mesh_metadata.MaterialGroupCount;i++)
    {
        std::cout << "IndexData " << i << ":" << std::endl;
        for(std::uint32_t j=0;j < material_groups[i].IndexCount ;j++)
        {
            std::cout << index_data[material_groups[i].FirstIndex + j] << "," << std::endl;
        }
    }
}

#endif // TEST_MESH_UTILS_H
