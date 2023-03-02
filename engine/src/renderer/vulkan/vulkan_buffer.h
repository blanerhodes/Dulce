#pragma once

#include "vulkan_types.inl"

b8 VulkanBufferCreate(VulkanContext* context, u64 size, VkBufferUsageFlags usage,
                      u32 memory_properity_flags, b8 bind_on_create, VulkanBuffer* out_buffer);

void VulkanBufferDestroy(VulkanContext* context, VulkanBuffer* buffer);

b8 VulkanBufferResize(VulkanContext* context, u64 new_size, VulkanBuffer* buffer, VkQueue queue, VkCommandPool pool);

void VulkanBufferBind(VulkanContext* context, VulkanBuffer* buffer, u64 offset);

void* VulkanBufferLockMemory(VulkanContext* context, VulkanBuffer* buffer, u64 offset, u64 size, u32 flags);
void VulkanBufferUnlockMemory(VulkanContext* context, VulkanBuffer* buffer);

void VulkanBufferLoadData(VulkanContext* context, VulkanBuffer* buffer, u64 offset, u64 size, u32 flags, void* data);

void VulkanBufferCopyTo(VulkanContext* context, VkCommandPool pool, VkFence fence, VkQueue queue,
                        VkBuffer source, u64 source_offset, VkBuffer dest, u64 dest_offset, u64 size);