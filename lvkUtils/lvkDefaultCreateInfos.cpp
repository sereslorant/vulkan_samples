
#include "lvkDefaultCreateInfos.h"


/*
 * Buffer
 */

const VkBufferCreateInfo LVK_EMPTY_BUFFER_CREATE_INFO =
{
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0x0,
    .size  = 0,
    .usage = 0x0,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount = 0,
    .pQueueFamilyIndices = 0,
};

/*
 * Image
 */

const VkImageCreateInfo LVK_EMPTY_2D_IMAGE_CREATE_INFO =
{
    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0x0,
    .imageType = VK_IMAGE_TYPE_2D,
    .format    = VK_FORMAT_UNDEFINED,
    .extent = 
    {
        .width  = 0,
        .height = 0,
        .depth  = 1,
    },
    .mipLevels     = 1,
    .arrayLayers   = 1,
    .samples       = VK_SAMPLE_COUNT_1_BIT,
    .tiling        = VK_IMAGE_TILING_OPTIMAL,
    .usage = 0x0,
    .sharingMode   = VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount = 0,
    .pQueueFamilyIndices = nullptr,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
};

/*
 * Image view
 */

const VkImageViewCreateInfo LVK_2D_IMG_VIEW_CREATE_INFO =
{
    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0x0,
    .image = VK_NULL_HANDLE,
    .viewType = VK_IMAGE_VIEW_TYPE_2D,
    .format   = VK_FORMAT_UNDEFINED,
    .components =
    {
        .r = VK_COMPONENT_SWIZZLE_IDENTITY,
        .g = VK_COMPONENT_SWIZZLE_IDENTITY,
        .b = VK_COMPONENT_SWIZZLE_IDENTITY,
        .a = VK_COMPONENT_SWIZZLE_IDENTITY,
    },
    .subresourceRange =
    {
        .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel   = 0,
        .levelCount     = 1,
        .baseArrayLayer = 0,
        .layerCount     = 1,
    }
};

const VkImageViewCreateInfo LVK_2D_ARRAY_IMG_VIEW_CREATE_INFO =
{
    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0x0,
    .image = VK_NULL_HANDLE,
    .viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY,
    .format   = VK_FORMAT_UNDEFINED,
    .components =
    {
        .r = VK_COMPONENT_SWIZZLE_IDENTITY,
        .g = VK_COMPONENT_SWIZZLE_IDENTITY,
        .b = VK_COMPONENT_SWIZZLE_IDENTITY,
        .a = VK_COMPONENT_SWIZZLE_IDENTITY,
    },
    .subresourceRange =
    {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel   = 0,
        .levelCount     = 1,
        .baseArrayLayer = 0,
        .layerCount     = 1,
    }
};

/*
 * Pipeline interface
 */

// Input state

const VkPipelineVertexInputStateCreateInfo LVK_EMPTY_INPUT_STATE_CREATE_INFO = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .vertexBindingDescriptionCount = 0,
    .pVertexBindingDescriptions = nullptr,
    .vertexAttributeDescriptionCount = 0,
    .pVertexAttributeDescriptions = nullptr,
};

// Pipeline layout

const VkPipelineLayoutCreateInfo LVK_EMPTY_PIPELINE_LAYOUT_CREATE_INFO {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0x0,
    .setLayoutCount = 0,
    .pSetLayouts = nullptr,
    .pushConstantRangeCount = 0,
    .pPushConstantRanges = nullptr,
};

/*
 * Pipeline fixed function state
 */

const VkPipelineInputAssemblyStateCreateInfo LVK_TRIANGLE_PRIMITIVE_ASSEMBLY_CREATE_INFO =
{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0x0,
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    .primitiveRestartEnable = VK_FALSE,
};

const VkPipelineViewportStateCreateInfo LVK_EMPTY_VIEWPORT_CREATE_INFO =
{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0x0,
    .viewportCount = 0,
    .pViewports    = nullptr,
    .scissorCount  = 0,
    .pScissors     = nullptr,
};

const VkPipelineRasterizationStateCreateInfo LVK_DEFAULT_RASTERIZER_CREATE_INFO =
{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0x0,
    .depthClampEnable        = VK_FALSE,
    .rasterizerDiscardEnable = VK_FALSE,
    .polygonMode             = VK_POLYGON_MODE_FILL,
    .cullMode                = VK_CULL_MODE_NONE,
    .frontFace               = VK_FRONT_FACE_CLOCKWISE,
    .depthBiasEnable         = VK_FALSE,
    .depthBiasConstantFactor = 0.0f,
    .depthBiasClamp          = 0.0f,
    .depthBiasSlopeFactor    = 0.0f,
    .lineWidth               = 1.0f,
};

const VkPipelineMultisampleStateCreateInfo LVK_DEFAULT_MULTISAMPLE_CREATE_INFO =
{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0x0,
    .rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
    .sampleShadingEnable   = VK_FALSE,
    .minSampleShading      = 1.0f,
    .pSampleMask           = nullptr,
    .alphaToCoverageEnable = VK_FALSE,
    .alphaToOneEnable      = VK_FALSE,
};

const VkPipelineColorBlendAttachmentState LVK_DEFAULT_COLOR_BLEND_ATTACHMENT_STATE =
{
    .blendEnable = VK_FALSE,
    .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
    .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
    .colorBlendOp   = VK_BLEND_OP_ADD,
    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
    .alphaBlendOp   = VK_BLEND_OP_ADD,
    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
};

const VkPipelineColorBlendStateCreateInfo LVK_DEFAULT_COLOR_BLEND_CREATE_INFO =
{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0x0,
    .logicOpEnable = VK_FALSE,
    .logicOp = VK_LOGIC_OP_COPY,
    .attachmentCount = 0,
    .pAttachments = nullptr,
    .blendConstants = {0.0f,0.0f,0.0f,0.0f,}
};

const VkPipelineDynamicStateCreateInfo LVK_EMPTY_DYNAMIC_STATE_CREATE_INFO =
{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0x0,
    .dynamicStateCount = 0,
    .pDynamicStates    = nullptr,
};

/*
 * Shader stage create info
 */

const VkPipelineShaderStageCreateInfo LVK_VERTEX_SHADER_STAGE_CREATE_INFO = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0x0,
    .stage = VK_SHADER_STAGE_VERTEX_BIT,
    .module = VK_NULL_HANDLE,
    .pName = nullptr,
    .pSpecializationInfo = nullptr,
};

const VkPipelineShaderStageCreateInfo LVK_FRAGMENT_SHADER_STAGE_CREATE_INFO = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0x0,
    .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
    .module = VK_NULL_HANDLE,
    .pName = nullptr,
    .pSpecializationInfo = nullptr,
};

/*
 * Pipeline create info
 */

const VkGraphicsPipelineCreateInfo LVK_EMPTY_PIPELINE_CREATE_INFO =
{
    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0x0,
    .stageCount          = 0,
    .pStages             = nullptr,
    .pVertexInputState   = &LVK_EMPTY_INPUT_STATE_CREATE_INFO,
    .pInputAssemblyState = nullptr,
    .pTessellationState  = nullptr,
    .pViewportState      = nullptr,
    .pRasterizationState = nullptr,
    .pMultisampleState   = nullptr,
    .pDepthStencilState  = nullptr,
    .pColorBlendState    = nullptr,
    .pDynamicState       = nullptr,
    .layout              = VK_NULL_HANDLE,
    .renderPass          = VK_NULL_HANDLE,
    .subpass             = 0,
    .basePipelineHandle  = VK_NULL_HANDLE,
    .basePipelineIndex   = -1,
};

/*
 * Descriptor pools
 */

const VkDescriptorPoolCreateInfo LVK_EMPTY_DESCRIPTOR_POOL_CREATE_INFO = {
    .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
    .pNext         = nullptr,
    .flags         = 0x0,
    .maxSets       = 0,
    .poolSizeCount = 0,
    .pPoolSizes    = VK_NULL_HANDLE
};

const VkSamplerCreateInfo LVK_DEFAULT_SAMPLER_CREATE_INFO = {
    .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0x0,

    .magFilter = VK_FILTER_NEAREST,
    .minFilter = VK_FILTER_NEAREST,
    .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,

    .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,

    .mipLodBias = 0.0f,

    .anisotropyEnable = VK_FALSE,
    .maxAnisotropy = 1,

    .compareEnable = VK_FALSE,
    .compareOp = VK_COMPARE_OP_ALWAYS,

    .minLod     = 0.0f,
    .maxLod     = 0.0f,

    .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,

    .unnormalizedCoordinates = VK_FALSE
};

const VkSamplerCreateInfo LVK_LINEAR_CLAMPED_SAMPLER_CREATE_INFO = {
    .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0x0,

    .magFilter = VK_FILTER_LINEAR,
    .minFilter = VK_FILTER_LINEAR,
    .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,

    .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
    .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
    .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,

    .mipLodBias = 0.0f,

    .anisotropyEnable = VK_FALSE,
    .maxAnisotropy = 1,

    .compareEnable = VK_FALSE,
    .compareOp = VK_COMPARE_OP_ALWAYS,

    .minLod     = 0.0f,
    .maxLod     = 0.0f,

    .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,

    .unnormalizedCoordinates = VK_FALSE
};

/*
 * Framebuffer
 */

const VkFramebufferCreateInfo LVK_EMPTY_FRAMEBUFFER_CREATE_INFO =
{
    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0x0,
    .renderPass      = VK_NULL_HANDLE,
    .attachmentCount = 0,
    .pAttachments    = nullptr,
    .width  = 0,
    .height = 0,
    .layers = 1,
};

/*
 * Render pass
 */

const VkAttachmentDescription LVK_EMPTY_ATTACHMENT_DESCRIPTION = {
    .flags          = 0x0,
    .format         = VK_FORMAT_UNDEFINED,
    .samples        = VK_SAMPLE_COUNT_1_BIT,
    .loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    .storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
    .finalLayout    = VK_IMAGE_LAYOUT_UNDEFINED
};

const VkAttachmentDescription LVK_PRESENT_CLEAR_ATTACHMENT_DESCRIPTION = {
    .flags          = 0x0,
    .format         = VK_FORMAT_UNDEFINED,
    .samples        = VK_SAMPLE_COUNT_1_BIT,
    .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
    .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
    .finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
};

const VkSubpassDescription LVK_EMPTY_SUBPASS_DESCRIPTION = {
    .flags                   = 0x0,
    .pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
    .inputAttachmentCount    = 0,
    .pInputAttachments       = nullptr,
    .colorAttachmentCount    = 0,
    .pColorAttachments       = nullptr,
    .pResolveAttachments     = nullptr,
    .pDepthStencilAttachment = nullptr,
    .preserveAttachmentCount = 0,
    .pPreserveAttachments    = nullptr
};

const VkAttachmentReference LVK_DEFAULT_COLOR_ATTACHMENT_REFERENCE = {
    .attachment = 0,
    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
};

const VkAttachmentReference LVK_DEFAULT_DEPTH_ATTACHMENT_REFERENCE = {
    .attachment = 0,
    .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
};

const VkSubpassDependency LVK_EMPTY_SUBPASS_DEPENDENCY = {
    .srcSubpass      = 0,
    .dstSubpass      = 0,
    .srcStageMask    = 0x0,
    .dstStageMask    = 0x0,
    .srcAccessMask   = 0x0,
    .dstAccessMask   = 0x0,
    .dependencyFlags = 0x0
};

const VkSubpassDependency LVK_DEFAULT_PREVIOUS_EXTERNAL_SUBPASS_DEPENDENCY = {
    .srcSubpass      = VK_SUBPASS_EXTERNAL,
    .dstSubpass      = 0,
    .srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    .dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    .srcAccessMask   = 0x0,
    .dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    .dependencyFlags = 0x0
};

const VkRenderPassCreateInfo LVK_EMPTY_RENDER_PASS_CREATE_INFO = {
    .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    .pNext           = nullptr,
    .flags           = 0x0,
    .attachmentCount = 0,
    .pAttachments    = nullptr,
    .subpassCount    = 0,
    .pSubpasses      = nullptr,
    .dependencyCount = 0,
    .pDependencies   = nullptr,
};

/*
 * Swapchain
 */

const VkSwapchainCreateInfoKHR LVK_DEFAULT_SWAPCHAIN_CREATE_INFO =
{
    .sType   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .pNext   = nullptr,
    .flags   = 0x0,
    .surface = VK_NULL_HANDLE,
    .minImageCount    = 0,
    .imageFormat      = VK_FORMAT_B8G8R8A8_UNORM,
    .imageColorSpace  = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
    .imageExtent      = {0,0},
    .imageArrayLayers = 1,
    .imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, // VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount = 0,
    .pQueueFamilyIndices   = nullptr,
    .preTransform   = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    .presentMode    = VK_PRESENT_MODE_FIFO_KHR,
    .clipped        = VK_TRUE,
    .oldSwapchain   = VK_NULL_HANDLE,
};
