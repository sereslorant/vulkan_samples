
import bpy; 
import bpy_extras;

import struct;

bl_info = {
    "name": "Binary mesh exporter",
    "category": "Import-Export",
};

def lGetUvTuple(uv,uv_merge_digits):
    Mlt = int(pow(10,uv_merge_digits));
    return (int(uv[0]*Mlt),int(uv[1]*Mlt));

def lSmoothNormalMesh(mesh,uv_merge_digits):
    VertUvIndices = {};
    MaterialGroupCount = 0;
    
    uv_data = mesh.uv_layers.active.data;
    NextId = 0;
    for polygon in mesh.polygons:
        if(MaterialGroupCount < polygon.material_index):
            MaterialGroupCount = polygon.material_index;
        
        for index_id in range(0,polygon.loop_total):
            loop_id = polygon.loop_start + index_id;
            UVKey = lGetUvTuple(uv_data[loop_id].uv,uv_merge_digits);
            
            Key = (polygon.vertices[index_id],UVKey);
            if(not (Key in VertUvIndices)):
                VertUvIndices[Key] = (NextId,loop_id);
                NextId += 1;
    
    MaterialGroupCount += 1;
    
    Vertices  = [];
    Normals   = [];
    TexCoords = [];
    
    MaterialGroups = [];
    for MaterialGroupId in range(0,MaterialGroupCount):
        MaterialGroups.append([]);
    
    for VertUv,IndexUv in VertUvIndices.items():
        VertexId,_ = VertUv;
        Id,UvIndex = IndexUv;
        
        Vertex = mesh.vertices[VertexId];
        
        Vertices.append([Vertex.co[0],Vertex.co[1],Vertex.co[2]]);
        Normals.append([Vertex.normal[0],Vertex.normal[1],Vertex.normal[2]]);
        TexCoords.append([uv_data[UvIndex].uv[0],uv_data[UvIndex].uv[1]]);
    
    for polygon in mesh.polygons:
        for index_id in range(0,polygon.loop_total):
            loop_id = polygon.loop_start + index_id;
            UVKey   = lGetUvTuple(uv_data[loop_id].uv,uv_merge_digits); 
            
            Index,_ = VertUvIndices[(polygon.vertices[index_id],UVKey)];
            MaterialGroups[polygon.material_index].append(Index);
    
    return {
        "Vertices":Vertices,
        "Normals":Normals,
        "TexCoords":TexCoords,
        "MaterialGroups":MaterialGroups,
        };

def lPerFaceNormalMesh(mesh,uv_merge_digits):
    VertUvIndices = {};
    MaterialGroupCount = 0;
    
    uv_data = mesh.uv_layers.active.data;
    NextId = 0;
    for polygon in mesh.polygons:
        if(MaterialGroupCount < polygon.material_index):
            MaterialGroupCount = polygon.material_index;
        
        for index_id in range(0,polygon.loop_total):
            loop_id = polygon.loop_start + index_id;
            UVKey = lGetUvTuple(uv_data[loop_id].uv,uv_merge_digits);
            
            Key = (polygon.vertices[index_id],polygon.index,UVKey);
            if(not (Key in VertUvIndices)):
                VertUvIndices[Key] = (NextId,loop_id);
                NextId += 1;
    
    MaterialGroupCount += 1;
    
    Vertices  = [];
    Normals   = [];
    TexCoords = [];
    
    MaterialGroups = [];
    for MaterialGroupId in range(0,MaterialGroupCount):
        MaterialGroups.append([]);
    
    for VertUv,IndexUv in VertUvIndices.items():
        VertexId,PolygonIndex,_ = VertUv;
        Id,UvIndex = IndexUv;
        
        Vertex = mesh.vertices[VertexId];
        Normal = mesh.polygons[PolygonIndex].normal;
        
        Vertices.append([Vertex.co[0],Vertex.co[1],Vertex.co[2]]);
        Normals.append([Normal[0],Normal[1],Normal[2]]);
        TexCoords.append([uv_data[UvIndex].uv[0],uv_data[UvIndex].uv[1]]);
    
    for polygon in mesh.polygons:
        for index_id in range(0,polygon.loop_total):
            loop_id = polygon.loop_start + index_id;
            UVKey   = lGetUvTuple(uv_data[loop_id].uv,uv_merge_digits); 
            
            Index,_ = VertUvIndices[(polygon.vertices[index_id],polygon.index,UVKey)];
            MaterialGroups[polygon.material_index].append(Index);
    
    return {
        "Vertices":Vertices,
        "Normals":Normals,
        "TexCoords":TexCoords,
        "MaterialGroups":MaterialGroups,
        };

DEINTERLEAVED_STR = "DEINTERLEAVED";
INTERLEAVED_STR   = "INTERLEAVED";

DEINTERLEAVED = int(0);
INTERLEAVED   = int(1);

STATIC_MESH   = int(0);
SKELETAL_MESH = int(1);

def lWriteToBinary(file_name,storage,processed_mesh):
    MeshType           = STATIC_MESH;
    VertexDataCount    = int(len(processed_mesh["Vertices"]));
    MaterialGroupCount = int(len(processed_mesh["MaterialGroups"]));
    
    with open(file_name,"wb") as out:
        out.write(storage.to_bytes(4,byteorder="little",signed=False));
        out.write(MeshType.to_bytes(4,byteorder="little",signed=False));
        out.write(VertexDataCount.to_bytes(4,byteorder="little",signed=False));
        out.write(MaterialGroupCount.to_bytes(4,byteorder="little",signed=False));
        for MaterialGroup in processed_mesh["MaterialGroups"]:
            out.write(len(MaterialGroup).to_bytes(4,byteorder="little",signed=False));
        
        if(storage == DEINTERLEAVED):
            for Vertex in processed_mesh["Vertices"]:
                out.write(struct.pack("fff",Vertex[0],Vertex[1],Vertex[2]));
            for Normal in processed_mesh["Normals"]:
                out.write(struct.pack("fff",Normal[0],Normal[1],Normal[2]));
            for TexCoord in processed_mesh["TexCoords"]:
                out.write(struct.pack("ff",TexCoord[0],TexCoord[1]));
        
        if(storage == INTERLEAVED):
            for VertexId in range(0,VertexDataCount):
                Vertex   = processed_mesh["Vertices"][VertexId];
                Normal   = processed_mesh["Normals"][VertexId];
                TexCoord = processed_mesh["TexCoords"][VertexId];
                out.write(struct.pack("ffffffff",Vertex[0],Vertex[1],Vertex[2]
                                                ,Normal[0],Normal[1],Normal[2]
                                                ,TexCoord[0],TexCoord[1]));
        
        for MaterialGroup in processed_mesh["MaterialGroups"]:
            for Index in MaterialGroup:
                out.write(Index.to_bytes(4,byteorder="little",signed=False));

def lExportMesh(filename,mesh,internal_format,smooth_normals,uv_merge_digits):
    
    if(smooth_normals):
        ProcessedMesh = lSmoothNormalMesh(mesh,uv_merge_digits);
    else:
        ProcessedMesh = lPerFaceNormalMesh(mesh,uv_merge_digits);
    
    if(internal_format == DEINTERLEAVED_STR):
        lWriteToBinary(filename,DEINTERLEAVED,ProcessedMesh);
    elif(internal_format == INTERLEAVED_STR):
        lWriteToBinary(filename,INTERLEAVED,ProcessedMesh);

class lBinaryExport(bpy.types.Operator,bpy_extras.io_utils.ExportHelper):
    bl_idname = "scene_export.lexport_binary";
    bl_label  = "Export to binary";
    
    filename_ext = "";
    
    internal_format = bpy.props.EnumProperty(
            items = 
                [
                    (INTERLEAVED_STR,"Interleaved","",),
                    (DEINTERLEAVED_STR,"Deinterleaved",""),
                ],
            name        = "Internal format",
            description = "",
            default     = INTERLEAVED_STR,
        );
    
    smooth_normals = bpy.props.BoolProperty(
            name = "Smooth normals",
            description = "",
            default = False,
        );
    
    def execute(self,context):
        
        selected_meshes = {};
        for scene_obj in context.selected_objects:
            selected_meshes[scene_obj.data.name] = scene_obj.data;
        
        if(len(selected_meshes.items()) == 1):
            for name,mesh in selected_meshes.items():
                lExportMesh(self.filepath,mesh,self.internal_format,self.smooth_normals,8);
        else:
            for name,mesh in selected_meshes.items():
                lExportMesh(self.filepath + "_" + mesh.name,mesh,self.internal_format,self.smooth_normals,8);
        
        return {"FINISHED"};

def lExportBinary(self,context):
    self.layout.operator(lBinaryExport.bl_idname,text="Binary");

def register():
    # Registering operators
    bpy.utils.register_class(lBinaryExport);
    
    # Registering menu items
    bpy.types.INFO_MT_file_export.append(lExportBinary);

def unregister():
    # Unregistering operators
    bpy.utils.unregister_class(lBinaryExport);
    
    # Unregistering menu items
    bpy.types.INFO_MT_file_export.remove(lExportBinary);

if(__name__ == "__main__"):
    register();
