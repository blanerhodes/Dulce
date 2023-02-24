#pragma once

#include "vulkan_types.inl"

b8 VulkanDeviceCreate(VulkanContext* context);

void VulkanDeviceDestroy(VulkanContext* context);

void VulkanDeviceQuerySwapchainSupport(
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    VulkanSwapchainSupportInfo* outSupportInfo);

b8 VulkanDeviceDetectDepthFormat(VulkanDevice* device);