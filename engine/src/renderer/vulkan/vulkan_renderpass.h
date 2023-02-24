#pragma once

#include "vulkan_types.inl"

void VulkanRenderPassCreate(
    VulkanContext* context,
    VulkanRenderPass* outRenderPass,
    f32 x, f32 y, f32 w, f32 h,
    f32 r, f32 g, f32 b, f32 a,
    f32 depth, u32 stencil);

void VulkanRenderPassDestroy(VulkanContext* context, VulkanRenderPass* renderPass);

void VulkanRenderPassBegin(
    VulkanCommandBuffer* commandBuffer,
    VulkanRenderPass* renderPass,
    VkFramebuffer frameBuffer);

void VulkanRenderPassEnd(VulkanCommandBuffer* commmandBuffer, VulkanRenderPass* renderPass);