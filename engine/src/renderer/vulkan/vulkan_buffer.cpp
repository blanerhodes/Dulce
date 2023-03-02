#include "vulkan_buffer.h"

#include "vulkan_device.h"
#include "vulkan_command_buffer.h"
#include "vulkan_utils.h"
#include "core/logger.h"
#include "core/dmemory.h"

b8 VulkanBufferCreate(VulkanContext* context, u64 size, VkBufferUsageFlags usage,
                      u32 memory_property_flags, b8 bind_on_create, VulkanBuffer* out_buffer) {
    DZeroMemory(out_buffer, sizeof(VulkanBuffer));
    out_buffer->total_size = size;
    out_buffer->usage = usage;
    out_buffer->memory_property_flags = memory_property_flags;

    VkBufferCreateInfo buffer_info = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    buffer_info.size = size;
    buffer_info.usage = usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(vkCreateBuffer(context->device.logical_device, &buffer_info, context->allocator, &out_buffer->handle));

    //Gather memory requirements
    VkMemoryRequirements requirements = {};
    vkGetBufferMemoryRequirements(context->device.logical_device, out_buffer->handle, &requirements);
    out_buffer->memory_index = context->FindMemoryIndex(requirements.memoryTypeBits, out_buffer->memory_property_flags);
    if (out_buffer->memory_index == -1) {
        DERROR("Unable to create vulkan buffer because the required memory type index was not found.");
        return false;
    }

    //Allocate memory info
    VkMemoryAllocateInfo allocate_info = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocate_info.allocationSize = requirements.size;
    allocate_info.memoryTypeIndex = (u32)out_buffer->memory_index;

    //Allocate memory
    VkResult result = vkAllocateMemory(context->device.logical_device, &allocate_info, context->allocator, &out_buffer->memory);
    if (result != VK_SUCCESS) {
        DERROR("Unable to create vulkan buffer because the required memory allocation failed. Error: %i", result);
        return false;
    }

    if (bind_on_create) {
        VulkanBufferBind(context, out_buffer, 0);
    }
    return true;
}

void VulkanBufferDestroy(VulkanContext* context, VulkanBuffer* buffer) {
    if (buffer->memory) {
        vkFreeMemory(context->device.logical_device, buffer->memory, context->allocator);
        buffer->memory = 0;
    }
    if (buffer->handle) {
        vkDestroyBuffer(context->device.logical_device, buffer->handle, context->allocator);
        buffer->handle = 0;
    }
    buffer->total_size = 0;
    buffer->usage = 0;
    buffer->is_locked = 0;
}

b8 VulkanBufferResize(VulkanContext* context, u64 new_size, VulkanBuffer* buffer, VkQueue queue, VkCommandPool pool) {
    VkBufferCreateInfo buffer_info = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    buffer_info.size = new_size;
    buffer_info.usage = buffer->usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkBuffer new_buffer = {};
    VK_CHECK(vkCreateBuffer(context->device.logical_device, &buffer_info, context->allocator, &new_buffer));

    //Gather memory reqs
    VkMemoryRequirements requirements = {};
    vkGetBufferMemoryRequirements(context->device.logical_device, new_buffer, &requirements);

    //Allocate memory info
    VkMemoryAllocateInfo allocate_info = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocate_info.allocationSize = requirements.size;
    allocate_info.memoryTypeIndex = (u32)buffer->memory_index;

    //Allocate memory
    VkDeviceMemory new_memory = {};
    VkResult result = vkAllocateMemory(context->device.logical_device, &allocate_info, context->allocator, &new_memory);
    if (result != VK_SUCCESS) {
        DERROR("Unable to resize vulkan buffer because the required memory allocation failed. Error: %i", result);
        return false;
    }
    //Bind the new buffer's memory
    VK_CHECK(vkBindBufferMemory(context->device.logical_device, new_buffer, new_memory, 0));

    //Copy over data
    VulkanBufferCopyTo(context, pool, 0, queue, buffer->handle, 0, new_buffer, 0, buffer->total_size);

    //Make sure anything potentially using thise is finished
    vkDeviceWaitIdle(context->device.logical_device);

    //Destroy old buffer
    if (buffer->memory) {
        vkFreeMemory(context->device.logical_device, buffer->memory, context->allocator);
        buffer->memory = 0;
    }
    if (buffer->handle) {
        vkDestroyBuffer(context->device.logical_device, buffer->handle, context->allocator);
        buffer->handle = 0;
    }
    buffer->total_size = new_size;
    buffer->memory = new_memory;
    buffer->handle = new_buffer;
    return true;
}

void VulkanBufferBind(VulkanContext* context, VulkanBuffer* buffer, u64 offset) {
    VK_CHECK(vkBindBufferMemory(context->device.logical_device, buffer->handle, buffer->memory, offset));
}

void* VulkanBufferLockMemory(VulkanContext* context, VulkanBuffer* buffer, u64 offset, u64 size, u32 flags) {
    void* data = 0;
    VK_CHECK(vkMapMemory(context->device.logical_device, buffer->memory, offset, size, flags, &data));
    return data;
}

void VulkanBufferUnlockMemory(VulkanContext* context, VulkanBuffer* buffer) {
    vkUnmapMemory(context->device.logical_device, buffer->memory);
}

void VulkanBufferLoadData(VulkanContext* context, VulkanBuffer* buffer, u64 offset, u64 size, u32 flags, void* data) {
    void* map_data = 0;
    VK_CHECK(vkMapMemory(context->device.logical_device, buffer->memory, offset, size, flags, &map_data));
    DCopyMemory(map_data, data, size);
    vkUnmapMemory(context->device.logical_device, buffer->memory);
}

void VulkanBufferCopyTo(VulkanContext* context, VkCommandPool pool, VkFence fence, VkQueue queue,
                        VkBuffer source, u64 source_offset, VkBuffer dest, u64 dest_offset, u64 size) {
    vkQueueWaitIdle(queue);
    VulkanCommandBuffer temp_command_buffer = {};
    VulkanCommandBufferAllocateAndBeginSingleUse(context, pool, &temp_command_buffer);

    //Prepare the copy command and add it to the command buffer
    VkBufferCopy copy_region = {};
    copy_region.srcOffset = source_offset;
    copy_region.dstOffset = dest_offset;
    copy_region.size = size;
    vkCmdCopyBuffer(temp_command_buffer.handle, source, dest, 1, &copy_region);

    VulkanCommandBufferEndSingleUse(context, pool, &temp_command_buffer, queue);
}