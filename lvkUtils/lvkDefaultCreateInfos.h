#ifndef LVK_DEF_CREATE_INFOS_H
#define LVK_DEF_CREATE_INFOS_H

#include <vulkan/vulkan.h>

/*
 * Buffer
 */

extern const VkBufferCreateInfo LVK_EMPTY_BUFFER_CREATE_INFO;

/*
 * Image
 */

extern const VkImageCreateInfo  LVK_EMPTY_2D_IMAGE_CREATE_INFO;

/*
 * Image view
 */

extern const VkImageViewCreateInfo LVK_2D_IMG_VIEW_CREATE_INFO;
extern const VkImageViewCreateInfo LVK_2D_ARRAY_IMG_VIEW_CREATE_INFO;

/*
 * Framebuffer
 */

extern const VkFramebufferCreateInfo LVK_EMPTY_FRAMEBUFFER_CREATE_INFO;

/*
 * Descriptor pools
 */

extern const VkDescriptorPoolCreateInfo LVK_EMPTY_DESCRIPTOR_POOL_CREATE_INFO;

extern const VkSamplerCreateInfo LVK_DEFAULT_SAMPLER_CREATE_INFO;
extern const VkSamplerCreateInfo LVK_LINEAR_CLAMPED_SAMPLER_CREATE_INFO;

/*
 * Pipeline interface
 */

// Input state

extern const VkPipelineVertexInputStateCreateInfo LVK_EMPTY_INPUT_STATE_CREATE_INFO;

// Pipeline layout

extern const VkPipelineLayoutCreateInfo LVK_EMPTY_PIPELINE_LAYOUT_CREATE_INFO;

/*
 * Pipeline fixed function state
 */

extern const VkPipelineInputAssemblyStateCreateInfo LVK_TRIANGLE_PRIMITIVE_ASSEMBLY_CREATE_INFO;

extern const VkPipelineViewportStateCreateInfo LVK_EMPTY_VIEWPORT_CREATE_INFO;

extern const VkPipelineRasterizationStateCreateInfo LVK_DEFAULT_RASTERIZER_CREATE_INFO;

extern const VkPipelineMultisampleStateCreateInfo LVK_DEFAULT_MULTISAMPLE_CREATE_INFO;

extern const VkPipelineColorBlendAttachmentState LVK_DEFAULT_COLOR_BLEND_ATTACHMENT_STATE;

extern const VkPipelineColorBlendStateCreateInfo LVK_DEFAULT_COLOR_BLEND_CREATE_INFO;

extern const VkPipelineDynamicStateCreateInfo LVK_EMPTY_DYNAMIC_STATE_CREATE_INFO;

/*
 * Shader stage create info
 */

extern const VkPipelineShaderStageCreateInfo LVK_VERTEX_SHADER_STAGE_CREATE_INFO;

extern const VkPipelineShaderStageCreateInfo LVK_FRAGMENT_SHADER_STAGE_CREATE_INFO;

/*
 * Pipeline create info
 */

extern const VkGraphicsPipelineCreateInfo LVK_EMPTY_PIPELINE_CREATE_INFO;

/*
 * Render pass
 */

extern const VkAttachmentDescription LVK_EMPTY_ATTACHMENT_DESCRIPTION;

extern const VkAttachmentDescription LVK_PRESENT_CLEAR_ATTACHMENT_DESCRIPTION;

extern const VkSubpassDescription LVK_EMPTY_SUBPASS_DESCRIPTION;

extern const VkAttachmentReference LVK_DEFAULT_COLOR_ATTACHMENT_REFERENCE;
extern const VkAttachmentReference LVK_DEFAULT_DEPTH_ATTACHMENT_REFERENCE;

extern const VkSubpassDependency LVK_EMPTY_SUBPASS_DEPENDENCY;

extern const VkSubpassDependency LVK_DEFAULT_PREVIOUS_EXTERNAL_SUBPASS_DEPENDENCY;

extern const VkRenderPassCreateInfo LVK_EMPTY_RENDER_PASS_CREATE_INFO;

/*
 * Swapchain
 */

extern const VkSwapchainCreateInfoKHR LVK_DEFAULT_SWAPCHAIN_CREATE_INFO;

#endif // LVK_DEF_CREATE_INFOS_H
