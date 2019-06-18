
#include "TestMeshUtils.h"




void TestLoadMeshGroup(const char *file_names[],unsigned int file_count)
{
    DummyMeshGroupAllocator MeshGroupAllocator;
    
    lrmIstreamMeshGroupLoader MeshGroupLoader(file_names,file_count);
    MeshGroupLoader.ReadMetadata();
    
    MeshGroupAllocator.AllocateMemory(MeshGroupLoader.StorageMode,MeshGroupLoader.MeshGroupSize);
    
    MeshGroupLoader.ReadData(MeshGroupAllocator);
    
    lrMeshGroup Meshes;
    MeshGroupLoader.ReadMeshes(Meshes);
    
    for(int i=0;i < MeshGroupLoader.GetMeshFiles().size();i++)
    {
        auto &MeshFile = MeshGroupLoader.GetMeshFiles()[i];
        auto &Mesh     = Meshes.Meshes[i];
        
        lrVertexDataFormat VertexDataFormat;
        VertexDataFormat.FillVertexDataFormat(MeshFile.MeshMetadata.Storage,MeshFile.MeshMetadata.VertexCount);
        float           *VertexData = MeshGroupAllocator.VertexData.get() + Meshes.IndexedSubMeshes[Mesh.FirstIndexedSubMesh].BaseVertex * 
                                      (lrmMeshMetadata::VERTEX_COMPONENT_COUNT + 
                                       lrmMeshMetadata::NORMAL_COMPONENT_COUNT + 
                                       lrmMeshMetadata::TEXCOORD_COMPONENT_COUNT);
      
        std::uint32_t   *IndexData  = MeshGroupAllocator.IndexData.get() + Meshes.IndexedSubMeshes[Mesh.FirstIndexedSubMesh].FirstIndex;
        
        PrintMetadata(MeshFile.MeshMetadata);
        PrintVertexData(MeshFile.MeshMetadata,VertexDataFormat,VertexData);
        PrintIndexData(MeshFile.MeshMetadata,&Meshes.IndexedSubMeshes[Mesh.FirstIndexedSubMesh],IndexData);
    }
}

int main(int argc, char *argv[])
{
    std::array<const char *,4> DeinterleavedFileNames =
    {
        "./TestMeshGroup/Cone_per_face_normals",
        "./TestMeshGroup/Cone_smooth_normals",
        "./TestMeshGroup/Cube_per_face_normals",
        "./TestMeshGroup/Cube_smooth_normals"
    };
    
    std::array<const char *,4> InterleavedFileNames =
    {
        "./TestMeshGroup/Cone_per_face_normals_interleaved",
        "./TestMeshGroup/Cone_smooth_normals_interleaved",
        "./TestMeshGroup/Cube_per_face_normals_interleaved",
        "./TestMeshGroup/Cube_smooth_normals_interleaved"
    };
    
    std::cout << "Deinterleaved meshes" << std::endl;
    TestLoadMeshGroup(DeinterleavedFileNames.data(),DeinterleavedFileNames.size());
    
    std::cout << "Interleaved meshes" << std::endl;
    TestLoadMeshGroup(InterleavedFileNames.data(),InterleavedFileNames.size());
    
    return 0;
}
