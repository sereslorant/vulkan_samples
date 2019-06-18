#ifndef LRM_ISTREAM_MESH_GROUP_LOADER_H
#define LRM_ISTREAM_MESH_GROUP_LOADER_H

#include <cinttypes>

#include <lvkRenderer/lvkResourceHierarchy.h>

struct lrmMeshMetadata
{
    static constexpr unsigned int VERTEX_COMPONENT_COUNT   = 3;
    static constexpr unsigned int NORMAL_COMPONENT_COUNT   = 3;
    static constexpr unsigned int TEXCOORD_COMPONENT_COUNT = 2;
    
    enum lrStorageMode
    {
        DEINTERLEAVED = 0,
        INTERLEAVED   = 1,
    };
    
    static constexpr unsigned int MAX_MATERIAL_GROUPS = 32;
    
    std::uint32_t Storage            = 0;
    std::uint32_t MeshType           = 0;
    std::uint32_t VertexCount        = 0;
    std::uint32_t MaterialGroupCount = 0;
    std::uint32_t IndexDataCount[MAX_MATERIAL_GROUPS] = {};
    
    std::size_t GetVertexSize() const
    {
        return VertexCount * VERTEX_COMPONENT_COUNT;
    }
    
    std::size_t GetNormalSize() const
    {
        return VertexCount * NORMAL_COMPONENT_COUNT;
    }
    
    std::size_t GetTexCoordSize() const
    {
        return VertexCount * TEXCOORD_COMPONENT_COUNT;
    }
    
    std::size_t GetTotalVertexDataSize() const
    {
        return GetVertexSize() + GetNormalSize() + GetTexCoordSize();
    }
    
    std::size_t GetTotalIndexDataSize() const
    {
        std::size_t TotalIndexDataSize = 0;
        for(std::uint32_t i=0;i < MaterialGroupCount;i++)
        {
            TotalIndexDataSize += IndexDataCount[i];
        }
        
        return TotalIndexDataSize;
    }
    
    void FillMatGroupRanges(lrMeshGroup::lrIndexedSubMesh indexed_sub_mesh[]) const
    {
        std::uint32_t LastOffset = 0;
        for(std::size_t i=0;i < MaterialGroupCount;i++)
        {
            indexed_sub_mesh[i].FirstIndex += LastOffset;
            indexed_sub_mesh[i].IndexCount = IndexDataCount[i];
            
            LastOffset += IndexDataCount[i];
        }
    }
};

union lVertexDataPtrs
{
    float *Interleaved;
    struct
    {
        float *Vertices;
        float *Normals;
        float *TexCoords;
    } Deinterleaved;
};

union lMeshGroupSize
{
    struct
    {
        std::uint32_t VertexDataSize;
        
        std::uint32_t IndexDataSize;
    } Interleaved;
    struct
    {
        std::uint32_t VertexDataSize;
        std::uint32_t NormalDataSize;
        std::uint32_t TexCoordDataSize;
        
        std::uint32_t IndexDataSize;
    } Deinterleaved;
    
    void AddMeshSize(lrmMeshMetadata &mesh_metadata)
    {
        switch(mesh_metadata.Storage)
        {
            case lrmMeshMetadata::INTERLEAVED:
                Interleaved.VertexDataSize += mesh_metadata.GetTotalVertexDataSize();
                
                Interleaved.IndexDataSize  += mesh_metadata.GetTotalIndexDataSize();
                break;
            case lrmMeshMetadata::DEINTERLEAVED:
                Deinterleaved.VertexDataSize   += mesh_metadata.GetVertexSize();
                Deinterleaved.NormalDataSize   += mesh_metadata.GetNormalSize();
                Deinterleaved.TexCoordDataSize += mesh_metadata.GetTexCoordSize();
                
                Deinterleaved.IndexDataSize += mesh_metadata.GetTotalIndexDataSize();
                break;
        }
    }
};

#include <array>
#include <fstream>

struct lrmIstreamMeshFile
{
    std::ifstream   In;
    lrmMeshMetadata MeshMetadata;
    
    void ReadMetadata()
    {
        In.read((char*)&MeshMetadata.Storage,sizeof(std::uint32_t));
        In.read((char*)&MeshMetadata.MeshType,sizeof(std::uint32_t));
        In.read((char*)&MeshMetadata.VertexCount,sizeof(std::uint32_t));
        In.read((char*)&MeshMetadata.MaterialGroupCount,sizeof(std::uint32_t));
        In.read((char*) MeshMetadata.IndexDataCount,MeshMetadata.MaterialGroupCount*sizeof(std::uint32_t));
    }
    
    void ReadVertexData(const lVertexDataPtrs &vertex_data)
    {
        switch(MeshMetadata.Storage)
        {
            case lrmMeshMetadata::INTERLEAVED:
                In.read((char*)vertex_data.Interleaved,MeshMetadata.GetTotalVertexDataSize() * sizeof(float));
                break;
                
            case lrmMeshMetadata::DEINTERLEAVED:
                In.read((char*)vertex_data.Deinterleaved.Vertices,  MeshMetadata.GetVertexSize() * sizeof(float));
                In.read((char*)vertex_data.Deinterleaved.Normals,   MeshMetadata.GetNormalSize() * sizeof(float));
                In.read((char*)vertex_data.Deinterleaved.TexCoords, MeshMetadata.GetTexCoordSize() * sizeof(float));
                break;
        }
    }
    
    void ReadIndexData(std::uint32_t *index_array)
    {
        In.read((char*)index_array,MeshMetadata.GetTotalIndexDataSize() * sizeof(std::uint32_t));
    }
};

class liAttribArrayAllocator
{
public:
    virtual lVertexDataPtrs RequestAttribArray(const lrmMeshMetadata &mesh_metadata) = 0;
    virtual std::uint32_t* RequestIndexArray(const lrmMeshMetadata &mesh_metadata) = 0;
    
    virtual ~liAttribArrayAllocator()
    {}
};

#include <vector>

class lrmIstreamMeshGroupLoader
{
private:
    std::vector<lrmIstreamMeshFile> MeshFiles;
    
public:
    
    lrmMeshMetadata::lrStorageMode StorageMode;
    lMeshGroupSize MeshGroupSize = {};
    
    std::int32_t TotalVertexDataSize = 0;
    std::int32_t TotalIndexDataSize = 0;
    
    std::size_t GetMeshCount()
    {
        return MeshFiles.size();
    }
    
    const std::vector<lrmIstreamMeshFile> &GetMeshFiles()
    {
        return MeshFiles;
    }
    
    void ReadMetadata()
    {
        for(auto &MeshFile : MeshFiles)
        {
            MeshFile.ReadMetadata();
            
            MeshGroupSize.AddMeshSize(MeshFile.MeshMetadata);
            
            switch(MeshFile.MeshMetadata.Storage)
            {
                case lrmMeshMetadata::INTERLEAVED:
                    StorageMode = lrmMeshMetadata::INTERLEAVED;
                    break;
                case lrmMeshMetadata::DEINTERLEAVED:
                    StorageMode = lrmMeshMetadata::DEINTERLEAVED;
                    break;
            }
        }
        
        switch(StorageMode)
        {
            case lrmMeshMetadata::INTERLEAVED:
                TotalVertexDataSize = MeshGroupSize.Interleaved.VertexDataSize;
                
                TotalIndexDataSize  = MeshGroupSize.Interleaved.IndexDataSize;
                break;
                
            case lrmMeshMetadata::DEINTERLEAVED:
                TotalVertexDataSize += MeshGroupSize.Deinterleaved.VertexDataSize;
                TotalVertexDataSize += MeshGroupSize.Deinterleaved.NormalDataSize;
                TotalVertexDataSize += MeshGroupSize.Deinterleaved.TexCoordDataSize;
                
                TotalIndexDataSize  =  MeshGroupSize.Deinterleaved.IndexDataSize;
                break;
        }
    }
    
    void ReadVertexData(liAttribArrayAllocator &attrib_array_allocator)
    {
        for(auto &MeshFile : MeshFiles)
        {
            MeshFile.ReadVertexData(attrib_array_allocator.RequestAttribArray(MeshFile.MeshMetadata));
        }
    }
    
    void ReadIndexData(liAttribArrayAllocator &attrib_array_allocator)
    {
        for(auto &MeshFile : MeshFiles)
        {
            MeshFile.ReadIndexData(attrib_array_allocator.RequestIndexArray(MeshFile.MeshMetadata));
        }
    }
    
    void ReadData(liAttribArrayAllocator &attrib_array_allocator)
    {
        for(auto &MeshFile : MeshFiles)
        {
            MeshFile.ReadVertexData(attrib_array_allocator.RequestAttribArray(MeshFile.MeshMetadata));
            MeshFile.ReadIndexData(attrib_array_allocator.RequestIndexArray(MeshFile.MeshMetadata));
        }
    }
    
    void ReadMeshes(lrMeshGroup &mesh_group)
    {
        std::int32_t FirstVertex = 0;
        std::int32_t FirstIndex  = 0;
        
        for(auto &MeshFile : MeshFiles)
        {
            mesh_group.Meshes.push_back({});
            
            mesh_group.Meshes.back().FirstIndexedSubMesh = mesh_group.IndexedSubMeshes.size();
            
            lrMeshGroup::lrIndexedSubMesh SubMeshTemplate = {};
            SubMeshTemplate.BaseVertex = FirstVertex;
            SubMeshTemplate.FirstIndex = FirstIndex;
            
            lrmMeshMetadata &MeshMetadata = MeshFile.MeshMetadata;
            
            std::array<lrMeshGroup::lrIndexedSubMesh,lrmMeshMetadata::MAX_MATERIAL_GROUPS> SubMeshes;
            for(std::size_t i=0;i < MeshMetadata.MaterialGroupCount;i++)
                {SubMeshes[i] = SubMeshTemplate;}
            
            MeshMetadata.FillMatGroupRanges(SubMeshes.data());
            
            for(std::size_t i=0;i < MeshMetadata.MaterialGroupCount;i++)
                {mesh_group.IndexedSubMeshes.push_back(SubMeshes[i]);}
            
            mesh_group.Meshes.back().IndexedSubMeshCount = mesh_group.IndexedSubMeshes.size() - mesh_group.Meshes.back().FirstIndexedSubMesh;
            
            FirstVertex += MeshFile.MeshMetadata.VertexCount;
            FirstIndex  += MeshFile.MeshMetadata.GetTotalIndexDataSize();
        }
    }
    
    void Close()
    {
        for(std::size_t i=0;i < MeshFiles.size();i++)
        {
            MeshFiles[i].In.close();
        }
    }
    
    lrmIstreamMeshGroupLoader(const char * const file_names[],unsigned int file_count)
    {
        MeshFiles.resize(file_count);
        for(std::size_t i=0;i < MeshFiles.size();i++)
        {
            MeshFiles[i].In.open(file_names[i]);
        }
    }
};

class lrmMeshGroupAllocator : public liAttribArrayAllocator
{
private:
    float         *CurrentVertexPtr   = nullptr;
    float         *CurrentNormalPtr   = nullptr;
    float         *CurrentTexCoordPtr = nullptr;
    std::uint32_t *CurrentIndexPtr    = nullptr;
    
public:
    
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
                Result.Deinterleaved.Normals   = CurrentNormalPtr;
                CurrentNormalPtr += mesh_metadata.GetNormalSize();
                Result.Deinterleaved.TexCoords = CurrentTexCoordPtr;
                CurrentTexCoordPtr += mesh_metadata.GetTexCoordSize();
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
    
    lrmMeshGroupAllocator(std::uint32_t storage,const lMeshGroupSize &mesh_group_size,void *vertex_begin,void *index_begin)
    {
        CurrentVertexPtr   = (float*)vertex_begin;
        if(storage == lrmMeshMetadata::DEINTERLEAVED)
        {
            CurrentNormalPtr   = CurrentVertexPtr + mesh_group_size.Deinterleaved.VertexDataSize;
            CurrentTexCoordPtr = CurrentNormalPtr + mesh_group_size.Deinterleaved.NormalDataSize;
        }
        
        CurrentIndexPtr = (std::uint32_t*)index_begin;
    }
    
    ~lrmMeshGroupAllocator()
    {}
};

#include <lvkResourceManager/lvkResourceGroupLoader.h>

struct lvkFileMeshGroupLoader : public liMeshGroupSrc
{
    lrmIstreamMeshGroupLoader MeshGroupLoader;
    
    virtual void LoadMetadata() override
    {
        MeshGroupLoader.ReadMetadata();
    }
    
    std::size_t GetVertexDataSize() const override
    {
        /*
        if(MeshGroupLoader.StorageMode == lrmMeshMetadata::DEINTERLEAVED)
            {std::cout << "Deinterleaved" << std::endl;}
        
        if(MeshGroupLoader.StorageMode == lrmMeshMetadata::INTERLEAVED)
            {std::cout << "Interleaved" << std::endl;}
        
        std::cout << "Vertex data size: " << MeshGroupLoader.TotalVertexDataSize << std::endl;
        */
        return MeshGroupLoader.TotalVertexDataSize * sizeof(float);
    }
    
    std::size_t GetIndexDataSize() const override
    {
        /*
        if(MeshGroupLoader.StorageMode == lrmMeshMetadata::DEINTERLEAVED)
            {std::cout << "Deinterleaved" << std::endl;}
        
        if(MeshGroupLoader.StorageMode == lrmMeshMetadata::INTERLEAVED)
            {std::cout << "Interleaved" << std::endl;}
        
        std::cout << "Index data size: " << MeshGroupLoader.TotalIndexDataSize << std::endl;
        */
        return MeshGroupLoader.TotalIndexDataSize * sizeof(std::uint32_t);
    }
    
    void ReadVertexOffsets(std::uint32_t &binding_count,std::uint32_t binding_offsets[],const std::uint32_t max_bindings) const override
    {
        if(MeshGroupLoader.StorageMode == lrmMeshMetadata::INTERLEAVED && max_bindings >= 1)
        {
            binding_offsets[0] = 0;
            binding_count      = 1;
        }
        else if(MeshGroupLoader.StorageMode == lrmMeshMetadata::DEINTERLEAVED && max_bindings >= 3)
        {
            binding_offsets[0] = 0;
            binding_offsets[1] = MeshGroupLoader.MeshGroupSize.Deinterleaved.VertexDataSize*sizeof(float);
            binding_offsets[2] = binding_offsets[1] + MeshGroupLoader.MeshGroupSize.Deinterleaved.NormalDataSize*sizeof(float);
            binding_count      = 3;
        }
    }
    
    void LoadVertexData(lMemoryView &vertex_data_region) override
    {
        lrmMeshGroupAllocator MeshGroupAllocator(
                                MeshGroupLoader.StorageMode,
                                MeshGroupLoader.MeshGroupSize,
                                vertex_data_region.GetPtr(),
                                nullptr
                               );
        
        MeshGroupLoader.ReadVertexData(MeshGroupAllocator);
        /*
        if(MeshGroupLoader.StorageMode == lrmMeshMetadata::DEINTERLEAVED)
            {std::cout << "Deinterleaved" << std::endl;}
        
        if(MeshGroupLoader.StorageMode == lrmMeshMetadata::INTERLEAVED)
            {std::cout << "Interleaved" << std::endl;}
        
        float *Vertices = (float *)vertex_data_region.GetPtr();
        for(int i=0;i < MeshGroupLoader.TotalVertexDataSize;i++)
        {
            std::cout << Vertices[i] << ";";
        }
        std::cout << std::endl;
        */
    }
    
    void LoadIndexData(lMemoryView &index_data_region) override
    {
        lrmMeshGroupAllocator MeshGroupAllocator(
                                MeshGroupLoader.StorageMode,
                                MeshGroupLoader.MeshGroupSize,
                                nullptr,
                                index_data_region.GetPtr()
                               );
        
        MeshGroupLoader.ReadIndexData(MeshGroupAllocator);
        /*
        if(MeshGroupLoader.StorageMode == lrmMeshMetadata::DEINTERLEAVED)
            {std::cout << "Deinterleaved" << std::endl;}
        
        if(MeshGroupLoader.StorageMode == lrmMeshMetadata::INTERLEAVED)
            {std::cout << "Interleaved" << std::endl;}
        
        std::uint32_t *Indices = (std::uint32_t *)index_data_region.GetPtr();
        for(int i=0;i < MeshGroupLoader.TotalIndexDataSize;i++)
        {
            std::cout << Indices[i] << ";";
        }
        std::cout << std::endl;
        */
    }
    
    virtual void FinishLoading(lrMeshGroup &mesh_group) override
    {
        MeshGroupLoader.ReadMeshes(mesh_group);
        MeshGroupLoader.Close();
    }
    
    lvkFileMeshGroupLoader(const char * const file_names[],unsigned int file_count)
        :MeshGroupLoader(file_names,file_count)
    {}
};

#endif // LRM_ISTREAM_MESH_GROUP_LOADER_H
