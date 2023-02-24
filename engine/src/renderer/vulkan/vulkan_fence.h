#pragma once
#include "vulkan_types.inl"

void VulkanFenceCreate(VulkanContext* context, b8 createSignaled, VulkanFence* outFence);

void VulkanFenceDestroy(VulkanContext* context, VulkanFence* fence);

b8 VulkanFenceWait(VulkanContext* context, VulkanFence* fence, u64 timeoutNs);

void VulkanFenceReset(VulkanContext* context, VulkanFence* fence);
