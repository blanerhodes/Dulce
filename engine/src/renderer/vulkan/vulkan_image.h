#pragma once

#include "vulkan_types.inl"

void VulkanImageCreate(
    VulkanContext* context,
    VkImageType imageType,
    u32 width,
    u32 height,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags memoryFlags,
    b32 createView,
    VkImageAspectFlags viewAspectFlags,
    VulkanImage* outImage);

void VulkanImageViewCreate(
    VulkanContext* context,
    VkFormat format,
    VulkanImage* image,
    VkImageAspectFlags aspectFlags);

//transition from old_layout to new_layout
void VulkanImageTransitionLayout(
    VulkanContext* context,
    VulkanCommandBuffer* command_buffer,
    VulkanImage* image,
    VkFormat format,
    VkImageLayout old_layout,
    VkImageLayout new_layout);

//copies data in buffer ot provided image
void VulkanImageCopyFromBuffer(
    VulkanContext* context,
    VulkanImage* image,
    VkBuffer buffer,
    VulkanCommandBuffer* command_buffer);

void VulkanImageDestroy(VulkanContext* context, VulkanImage* image);