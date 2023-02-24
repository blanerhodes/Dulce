#pragma once
#include "vulkan_types.inl"

void VulkanFrameBufferCreate(
    VulkanContext* context,
    VulkanRenderPass* renderpass,
    u32 width, u32 height, u32 attachmentCount,
    VkImageView* attachments,
    VulkanFrameBuffer* outFrameBuffer);

void VulkanFrameBufferDestroy(VulkanContext* context, VulkanFrameBuffer* frameBuffer);