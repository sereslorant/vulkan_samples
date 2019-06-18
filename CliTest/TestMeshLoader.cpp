
#include "TestMeshUtils.h"

#include <fstream>

void TestReadMesh(lrmIstreamMeshFile &mesh_file)
{
    DummyMeshGroupAllocator AttribArrayAllocator;
    
    lrmMeshMetadata::lrStorageMode StorageMode;
    lMeshGroupSize MeshGroupSize = {};
    
    mesh_file.ReadMetadata();
    
    MeshGroupSize.AddMeshSize(mesh_file.MeshMetadata);
    
    switch(mesh_file.MeshMetadata.Storage)
    {
        case lrmMeshMetadata::INTERLEAVED:
            StorageMode = lrmMeshMetadata::INTERLEAVED;
            break;
        case lrmMeshMetadata::DEINTERLEAVED:
            StorageMode = lrmMeshMetadata::DEINTERLEAVED;
            break;
    }
    
    AttribArrayAllocator.AllocateMemory(StorageMode,MeshGroupSize);
    
    mesh_file.ReadVertexData(AttribArrayAllocator.RequestAttribArray(mesh_file.MeshMetadata));
    mesh_file.ReadIndexData(AttribArrayAllocator.RequestIndexArray(mesh_file.MeshMetadata));
    
    lrVertexDataFormat VertexDataFormat;
    //std::array<lrBufferRange,lrmMeshMetadata::MAX_MATERIAL_GROUPS> MaterialGroups;
    std::array<lrMeshGroup::lrIndexedSubMesh,lrmMeshMetadata::MAX_MATERIAL_GROUPS> MaterialGroups;
    
    //mesh_file.MeshMetadata.FillVertexDataFormat(VertexDataFormat);
    VertexDataFormat.FillVertexDataFormat(mesh_file.MeshMetadata.Storage,mesh_file.MeshMetadata.VertexCount);
    mesh_file.MeshMetadata.FillMatGroupRanges(&MaterialGroups[0]);
    
    PrintMetadata(mesh_file.MeshMetadata);
    PrintVertexData(mesh_file.MeshMetadata,VertexDataFormat, AttribArrayAllocator.VertexData.get());
    PrintIndexData(mesh_file.MeshMetadata,&MaterialGroups[0],AttribArrayAllocator.IndexData.get());
}

int main(int argc, char *argv[])
{
    {
        lrmIstreamMeshFile MeshFile;
        
        MeshFile.In.open("TestMesh/TestStaticMeshDeinterleaved",std::ifstream::binary);
        if(MeshFile.In.is_open())
            {TestReadMesh(MeshFile);}
    }
    
    {
        lrmIstreamMeshFile MeshFile;
        
        MeshFile.In.open("TestMesh/TestStaticMeshInterleaved",std::ifstream::binary);
        if(MeshFile.In.is_open())
            {TestReadMesh(MeshFile);}
    }
    
    std::int32_t DeinterleavedFileCount = 4;
    const char *DeinterleavedFileNames[4] =
    {
        "Cone_per_face_normals",
        "Cone_smooth_normals",
        "Cube_per_face_normals",
        "Cube_smooth_normals"
    };
    
    for(int i=0;i < DeinterleavedFileCount;i++)
    {
        lrmIstreamMeshFile MeshFile;
        MeshFile.In.open(DeinterleavedFileNames[i],std::ifstream::binary);
        
        if(MeshFile.In.is_open())
            {TestReadMesh(MeshFile);}
    }
    
    std::int32_t InterleavedFileCount = 4;
    const char *InterleavedFileNames[4] =
    {
        "Cone_per_face_normals_interleaved",
        "Cone_smooth_normals_interleaved",
        "Cube_per_face_normals_interleaved",
        "Cube_smooth_normals_interleaved"
    };
    
    for(int i=0;i < InterleavedFileCount;i++)
    {
        lrmIstreamMeshFile MeshFile;
        MeshFile.In.open(InterleavedFileNames[i],std::ifstream::binary);
        
        if(MeshFile.In.is_open())
            {TestReadMesh(MeshFile);}
    }
    
    return 0;
}
