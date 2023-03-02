#pragma once

#include "vulkan_types.inl"

void VulkanSwapchainCreate(VulkanContext* context, u32 width, u32 height, VulkanSwapchain* out_swapchain);

void VulkanSwapchainRecreate(VulkanContext* context, u32 width, u32 height, VulkanSwapchain* swapchain);

void VulkanSwapchainDestroy(VulkanContext* context, VulkanSwapchain* swapchain);

b8 VulkanSwapchainAcquireNextImageIndex(VulkanContext* context, VulkanSwapchain* swapchain, u64 timeout_ns,
                                        VkSemaphore image_available_semaphore, VkFence fence, u32* out_image_index);

void VulkanSwapchainPresent(VulkanContext* context, VulkanSwapchain* swapchain, VkQueue graphics_queue,
                            VkQueue present_queue, VkSemaphore render_complete_semaphore, u32 present_image_index);