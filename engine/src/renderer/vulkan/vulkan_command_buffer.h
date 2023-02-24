#pragma once

#include "vulkan_types.inl"

void VulkanCommandBufferAllocate(
    VulkanContext* context,
    VkCommandPool pool,
    b8 isPrimary,
    VulkanCommandBuffer* outCommandBuffer);

void VulkanCommandBufferFree(
    VulkanContext* context,
    VkCommandPool pool,
    VulkanCommandBuffer* commandBuffer);

void VulkanCommandBufferBegin(
    VulkanCommandBuffer* commandBuffer,
    b8 isSingleUse,
    b8 isRenderpassContinue,
    b8 isSimultaneousUse);

void VulkanCommandBufferEnd(VulkanCommandBuffer* commandBuffer);

void VulkanCommandBufferUpdateSubmitted(VulkanCommandBuffer* commandBuffer);

void VulkanCommandBufferReset(VulkanCommandBuffer* commandBuffer);

//Allocates and begins recording to outCommandBuffer
void VulkanCommandBufferAllocateAndBeginSingleUse(
    VulkanContext* context,
    VkCommandPool pool,
    VulkanCommandBuffer* outCommandBuffer);

//End recording, submits to and waits for queue operation and frees the provided command buffer
void VulkanCommandBufferEndSingleUse(
    VulkanContext* context,
    VkCommandPool pool,
    VulkanCommandBuffer* commandBuffer,
    VkQueue queue);