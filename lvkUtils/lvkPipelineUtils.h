#ifndef LVK_PIPELINE_UTILS_H
#define LVK_PIPELINE_UTILS_H

#include <cinttypes>

#include <lvkUtils/lvkDefaultCreateInfos.h>

struct lvkPipelineDynamicState
{
    VkPipelineDynamicStateCreateInfo DynamicStateCreateInfo                     = LVK_EMPTY_DYNAMIC_STATE_CREATE_INFO;
    VkDynamicState                   DynamicStates[VK_DYNAMIC_STATE_RANGE_SIZE] = {};
    
    void AddDynamicState(VkDynamicState dynamic_state)
    {
        bool Found = false;
        for(std::uint32_t i=0;i < DynamicStateCreateInfo.dynamicStateCount;i++)
        {
            if(DynamicStates[i] == dynamic_state)
                {Found = true;}
        }
        
        if(!Found)
        {
            DynamicStates[DynamicStateCreateInfo.dynamicStateCount] = dynamic_state;
            DynamicStateCreateInfo.dynamicStateCount++;
        }
    }
    
    void AssignToPipelineCreateInfo(VkGraphicsPipelineCreateInfo &pipeline_create_info)
    {
        DynamicStateCreateInfo.pDynamicStates = DynamicStates;
        if(DynamicStateCreateInfo.dynamicStateCount > 0)
            {pipeline_create_info.pDynamicState = &DynamicStateCreateInfo;}
    }
};

struct lvkPipelineFixedFunctionState
{
    VkViewport Viewport = {};
    VkRect2D   Scissor  = {};
    
    VkPipelineInputAssemblyStateCreateInfo InputAssemblyStateCreateInfo = LVK_TRIANGLE_PRIMITIVE_ASSEMBLY_CREATE_INFO;
    VkPipelineViewportStateCreateInfo      ViewportStateCreateInfo      = LVK_EMPTY_VIEWPORT_CREATE_INFO;
    VkPipelineRasterizationStateCreateInfo RasterizationStateCreateInfo = LVK_DEFAULT_RASTERIZER_CREATE_INFO;
    VkPipelineMultisampleStateCreateInfo   MultisampleStateCreateInfo   = LVK_DEFAULT_MULTISAMPLE_CREATE_INFO;
    
    void SetViewport(float x,float y,float width,float height)
    {
        Viewport.x = x;
        Viewport.y = y;
        Viewport.width = width;
        Viewport.height = height;
        Viewport.minDepth = 0.0f;
        Viewport.maxDepth = 1.0f;
    }
    
    void SetScissor(std::int32_t x,std::int32_t y,std::uint32_t width,std::uint32_t height)
    {
        Scissor.offset = {x, y};
        Scissor.extent.width  = width;
        Scissor.extent.height = height;
    }
    
    void AssignToPipelineCreateInfo(VkGraphicsPipelineCreateInfo &pipeline_create_info)
    {
        ViewportStateCreateInfo.viewportCount = 1;
        ViewportStateCreateInfo.pViewports    = &Viewport;
        ViewportStateCreateInfo.scissorCount  = 1;
        ViewportStateCreateInfo.pScissors     = &Scissor;
        
        pipeline_create_info.pInputAssemblyState = &InputAssemblyStateCreateInfo;
        pipeline_create_info.pViewportState = &ViewportStateCreateInfo;
        pipeline_create_info.pRasterizationState = &RasterizationStateCreateInfo;
        pipeline_create_info.pMultisampleState = &MultisampleStateCreateInfo;
    }
};

class lvkVertexInputDescriptionBase
{
protected:

    VkPipelineVertexInputStateCreateInfo VertexInputStateCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .vertexBindingDescriptionCount = 0,
        .pVertexBindingDescriptions = nullptr,
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions = nullptr,
    };

public:
    
    void AssignToPipelineCreateInfo(VkGraphicsPipelineCreateInfo &pipeline_create_info)
    {
        pipeline_create_info.pVertexInputState = &VertexInputStateCreateInfo;
    }
};

class lvkVertexInputDescription : public lvkVertexInputDescriptionBase
{
private:
    static constexpr unsigned int MAX_INPUT_BINDINGS = 16;
    static constexpr unsigned int MAX_ATTRIBUTES = 16;

    VkVertexInputBindingDescription VertexInputBindingDescriptions[MAX_INPUT_BINDINGS] = {};
    VkVertexInputAttributeDescription VertexInputAttributeDescriptions[MAX_ATTRIBUTES] = {};

public:

    void AddVertexInputBinding(uint32_t binding,uint32_t stride,VkVertexInputRate input_rate)
    {
        VertexInputBindingDescriptions[VertexInputStateCreateInfo.vertexBindingDescriptionCount].binding = binding;
        VertexInputBindingDescriptions[VertexInputStateCreateInfo.vertexBindingDescriptionCount].stride = stride;
        VertexInputBindingDescriptions[VertexInputStateCreateInfo.vertexBindingDescriptionCount].inputRate = input_rate;
        
        VertexInputStateCreateInfo.pVertexBindingDescriptions = VertexInputBindingDescriptions;
        VertexInputStateCreateInfo.vertexBindingDescriptionCount++;
    }

    void AddVertexInputAttributeDescription(uint32_t binding,uint32_t location,VkFormat format,uint32_t offset)
    {
        VertexInputAttributeDescriptions[VertexInputStateCreateInfo.vertexAttributeDescriptionCount].binding = binding;
        VertexInputAttributeDescriptions[VertexInputStateCreateInfo.vertexAttributeDescriptionCount].location = location;
        VertexInputAttributeDescriptions[VertexInputStateCreateInfo.vertexAttributeDescriptionCount].format = format;
        VertexInputAttributeDescriptions[VertexInputStateCreateInfo.vertexAttributeDescriptionCount].offset = offset;
        
        VertexInputStateCreateInfo.pVertexAttributeDescriptions = VertexInputAttributeDescriptions;
        VertexInputStateCreateInfo.vertexAttributeDescriptionCount++;
    }
};

#endif // LVK_PIPELINE_UTILS_H
