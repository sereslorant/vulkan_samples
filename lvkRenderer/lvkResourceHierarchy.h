#ifndef LVK_RESOURCE_HIERARCHY_H
#define LVK_RESOURCE_HIERARCHY_H

#include <vulkan/vulkan.h>

#include <array>
#include <vector>

/*
 * Rendering resources
 */

struct lvkDescriptorSet
{
    VkPipelineLayout PipelineLayout;
    VkDescriptorSet  DescriptorSet;
    
    void BindDescriptorSet(VkCommandBuffer command_buffer) const
    {
        vkCmdBindDescriptorSets(command_buffer,VK_PIPELINE_BIND_POINT_GRAPHICS,PipelineLayout,1,1,&DescriptorSet,0,nullptr);
    }
};

struct lvkDynamicDescriptorSet
{
    VkPipelineLayout PipelineLayout;
    VkDescriptorSet  DescriptorSet;
    std::uint32_t    Offset;
    std::uint32_t    StructSize;
    
    void BindDescriptorSet(VkCommandBuffer command_buffer,std::uint32_t struct_id) const
    {
        std::uint32_t DynamicOffset = Offset + struct_id*StructSize;
        
        vkCmdBindDescriptorSets(command_buffer,VK_PIPELINE_BIND_POINT_GRAPHICS,PipelineLayout,0,1,&DescriptorSet,1,&DynamicOffset);
    }
};

class lvkMeshGroupVertexData
{
public:
    constexpr static unsigned int MAX_BUFFERS = 16;
    
private:
    std::uint32_t                        BindingCount;
    std::array<VkBuffer,MAX_BUFFERS>     VertexBuffers;
    std::array<VkDeviceSize,MAX_BUFFERS> VertexBufferOffsets;
    
public:
    
    void BindVertexBuffers(VkCommandBuffer command_buffer) const
    {
        vkCmdBindVertexBuffers(command_buffer,0,BindingCount,VertexBuffers.data(),VertexBufferOffsets.data());
    }
    
    lvkMeshGroupVertexData(unsigned int binding_count = 0,const std::array<VkBuffer,MAX_BUFFERS> &vertex_buffers = {},const std::array<VkDeviceSize,MAX_BUFFERS> &vertex_buffer_offsets = {})
        :BindingCount(binding_count),VertexBuffers(vertex_buffers),VertexBufferOffsets(vertex_buffer_offsets)
    {}
};

class lvkMeshGroupIndexData
{
private:
    VkBuffer     IndexBuffer;
    VkDeviceSize IndexBufferOffset;
    VkIndexType  IndexType;
    
public:
    
    void BindIndexBuffer(VkCommandBuffer command_buffer) const
    {
        vkCmdBindIndexBuffer(command_buffer,IndexBuffer,IndexBufferOffset,IndexType);
    }
    
    lvkMeshGroupIndexData(VkBuffer index_buffer = nullptr,VkDeviceSize index_buffer_offset = 0,VkIndexType index_type = VK_INDEX_TYPE_UINT32)
        :IndexBuffer(index_buffer),IndexBufferOffset(index_buffer_offset),IndexType(index_type)
    {}
};

struct lrMeshGroup
{
    struct lrMesh
    {
        std::size_t FirstSubMesh;
        std::size_t SubMeshCount;
        
        std::size_t FirstIndexedSubMesh;
        std::size_t IndexedSubMeshCount;
    };
    
    struct lrSubMesh
    {
        std::uint32_t BaseVertex;
        std::uint32_t VertexCount;
    };
    
    struct lrIndexedSubMesh
    {
        std::uint32_t BaseVertex;
        std::uint32_t FirstIndex;
        std::uint32_t IndexCount;
    };
    
    std::vector<lrMesh>           Meshes;
    std::vector<lrSubMesh>        SubMeshes;
    std::vector<lrIndexedSubMesh> IndexedSubMeshes;
};

/*
 * Resource hierarchy
 */

struct lrResourceNode;
struct lrPipelineState;
struct lrUniformBindings;
struct lrDynamicUniformBindings;
struct lrMeshGroupBindings;
struct lrMeshDrawCall;

class liResourceHierarchyVisitor
{
public:
    virtual void Visit(const lrResourceNode &node)           = 0;
    virtual void Visit(const lrPipelineState &node)          = 0;
    virtual void Visit(const lrUniformBindings &node)        = 0;
    virtual void Visit(const lrDynamicUniformBindings &node) = 0;
    virtual void Visit(const lrMeshGroupBindings &node)      = 0;
    virtual void Visit(const lrMeshDrawCall &node)           = 0;
    
    virtual ~liResourceHierarchyVisitor()
    {}
};

struct lrResourceNode
{
    std::vector<lrResourceNode *> Children;
    
    virtual void Accept(liResourceHierarchyVisitor &visitor) const
    {
        visitor.Visit(*this);
    }
};

struct lrPipelineState : public lrResourceNode
{
    VkPipeline Pipeline = nullptr;
    
    virtual void Accept(liResourceHierarchyVisitor &visitor) const override
    {
        visitor.Visit(*this);
    }
};

struct lrUniformBindings : public lrResourceNode
{
    const lvkDescriptorSet *UniformBindings = nullptr;
    
    virtual void Accept(liResourceHierarchyVisitor &visitor) const override
    {
        visitor.Visit(*this);
    }
};

struct lrDynamicUniformBindings : public lrResourceNode
{
    const lvkDynamicDescriptorSet *DynamicUniformBindings = nullptr;
    
    virtual void Accept(liResourceHierarchyVisitor &visitor) const override
    {
        visitor.Visit(*this);
    }
};

struct lrMeshGroupBindings : public lrResourceNode
{
    const lvkMeshGroupVertexData *VertexData = nullptr;
    const lvkMeshGroupIndexData  *IndexData  = nullptr;
    const lrMeshGroup            *MeshGroup  = nullptr;
    
    virtual void Accept(liResourceHierarchyVisitor &visitor) const override
    {
        visitor.Visit(*this);
    }
};

struct lrMeshDrawCall : public lrResourceNode
{
    bool        Indexed;
    std::size_t MeshId;
    
    //lrMeshGroup::lrIndexedSubMesh IndexedSubMesh;
    
    virtual void Accept(liResourceHierarchyVisitor &visitor) const override
    {
        visitor.Visit(*this);
    }
};

class lvkHierarchyRecorder : public liResourceHierarchyVisitor
{
private:
    VkCommandBuffer CommandBuffer;
    
    const lrMeshGroup *MeshGroup = nullptr;
    
    void VisitChildren(const lrResourceNode &node)
    {
        for(auto Child : node.Children)
        {
            Child->Accept(*this);
        }
    }
    
public:
    
    virtual void Visit(const lrResourceNode &node) override
    {
        VisitChildren(node);
    }
    
    virtual void Visit(const lrPipelineState &node) override
    {
        VkPipeline Pipeline = node.Pipeline;
        vkCmdBindPipeline(CommandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,Pipeline);
        
        VisitChildren(node);
    }
    
    virtual void Visit(const lrUniformBindings &node) override
    {
        const lvkDescriptorSet *UniformBindings = node.UniformBindings;
        UniformBindings->BindDescriptorSet(CommandBuffer);
        
        VisitChildren(node);
    }
    
    virtual void Visit(const lrDynamicUniformBindings &node) override
    {
        const lvkDynamicDescriptorSet *DynamicUniformBindings = node.DynamicUniformBindings;
        
        std::size_t CurrentId = 0;
        
        // Kísérleezés
        DynamicUniformBindings->BindDescriptorSet(CommandBuffer,CurrentId);
        
        for(auto Child : node.Children)
        {
            DynamicUniformBindings->BindDescriptorSet(CommandBuffer,CurrentId);
            Child->Accept(*this);
            CurrentId++;
        }
    }
    
    virtual void Visit(const lrMeshGroupBindings &node) override
    {
        if(node.VertexData != nullptr)
        {
            const lvkMeshGroupVertexData *VertexData = node.VertexData;
            VertexData->BindVertexBuffers(CommandBuffer);
        }
        
        if(node.IndexData != nullptr)
        {
            const lvkMeshGroupIndexData *IndexData = node.IndexData;
            IndexData->BindIndexBuffer(CommandBuffer);
        }
        
        if(node.MeshGroup != nullptr)
        {
            MeshGroup = node.MeshGroup;
        }
        
        VisitChildren(node);
    }
    
    virtual void Visit(const lrMeshDrawCall &node) override
    {
        if(MeshGroup != nullptr)
        {
            if(node.Indexed)
            {
                vkCmdDrawIndexed(
                    CommandBuffer,
                    MeshGroup->IndexedSubMeshes[node.MeshId].IndexCount,
                    1,
                    MeshGroup->IndexedSubMeshes[node.MeshId].FirstIndex,
                    MeshGroup->IndexedSubMeshes[node.MeshId].BaseVertex,
                    0
                );
                /*
                vkCmdDrawIndexed(
                    CommandBuffer,
                    node.IndexedSubMesh.IndexCount,
                    1,
                    node.IndexedSubMesh.FirstIndex,
                    node.IndexedSubMesh.BaseVertex,
                    0
                );
                */
            }
            else
            {
                vkCmdDraw(
                    CommandBuffer,
                    MeshGroup->SubMeshes[node.MeshId].VertexCount,
                    1,
                    MeshGroup->SubMeshes[node.MeshId].BaseVertex,
                    0
                );
            }
        }
        
        VisitChildren(node);
    }
    
    lvkHierarchyRecorder(VkCommandBuffer command_buffer)
        :CommandBuffer(command_buffer)
    {}
    
    virtual ~lvkHierarchyRecorder() override
    {}
};

#endif // LVK_RESOURCE_HIERARCHY_H
