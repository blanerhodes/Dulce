#pragma once

#include "vulkan_types.inl"

b8 VulkanGraphicsPipelineCreate(
    VulkanContext* context,
    VulkanRenderPass* render_pass,
    u32 attribute_count,
    VkVertexInputAttributeDescription* attributes,
    u32 descriptor_set_layout_count,
    VkDescriptorSetLayout* descriptor_set_layouts,
    u32 stage_count,
    VkPipelineShaderStageCreateInfo* stages,
    VkViewport viewport,
    VkRect2D scissor,
    b8 is_wireframe,
    VulkanPipeline* out_pipeline);

void VulkanPipelineDestroy(VulkanContext* context, VulkanPipeline* pipeline);

void VulkanPipelineBind(VulkanCommandBuffer* command_buffer, VkPipelineBindPoint bind_point, VulkanPipeline* pipeline);