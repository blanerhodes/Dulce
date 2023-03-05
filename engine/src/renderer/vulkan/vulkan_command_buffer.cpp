#include "vulkan_command_buffer.h"
#include "core/dmemory.h"


void VulkanCommandBufferAllocate(VulkanContext* context, VkCommandPool pool, b8 is_primary,
                                 VulkanCommandBuffer* out_command_buffer) {

    DZeroMemory(out_command_buffer, sizeof(VulkanCommandBuffer));

    VkCommandBufferAllocateInfo allocate_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    allocate_info.commandPool = pool;
    allocate_info.level = is_primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    allocate_info.commandBufferCount = 1;
    allocate_info.pNext = 0;

    out_command_buffer->state = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
    VK_CHECK(vkAllocateCommandBuffers(
        context->device.logical_device,
        &allocate_info,
        &out_command_buffer->handle));
    out_command_buffer->state = COMMAND_BUFFER_STATE_READY;
}

void VulkanCommandBufferFree(VulkanContext* context, VkCommandPool pool, VulkanCommandBuffer* command_buffer) {
    vkFreeCommandBuffers(context->device.logical_device, pool, 1, &command_buffer->handle);
    command_buffer->handle = 0;
    command_buffer->state = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
}

void VulkanCommandBufferBegin(VulkanCommandBuffer* command_buffer, b8 is_single_use, 
                              b8 is_renderpass_continue, b8 is_simultaneous_use) {
    VkCommandBufferBeginInfo begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    begin_info.flags = 0;
    if (is_single_use) {
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    }
    if (is_renderpass_continue) {
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    }
    if (is_simultaneous_use) {
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    }
    VK_CHECK(vkBeginCommandBuffer(command_buffer->handle, &begin_info));
    command_buffer->state = COMMAND_BUFFER_STATE_RECORDING;
}

void VulkanCommandBufferEnd(VulkanCommandBuffer* command_buffer) {
    VK_CHECK(vkEndCommandBuffer(command_buffer->handle));
    command_buffer->state = COMMAND_BUFFER_STATE_RECORDING_ENDED;
}

void VulkanCommandBufferUpdateSubmitted(VulkanCommandBuffer* command_buffer) {
    command_buffer->state = COMMAND_BUFFER_STATE_SUBMITTED;
}

void VulkanCommandBufferReset(VulkanCommandBuffer* command_buffer) {
    command_buffer->state = COMMAND_BUFFER_STATE_READY;
}

//Allocates and begins recording to outCommandBuffer
void VulkanCommandBufferAllocateAndBeginSingleUse(VulkanContext* context, VkCommandPool pool,
                                                  VulkanCommandBuffer* out_command_buffer) {
    VulkanCommandBufferAllocate(context, pool, true, out_command_buffer);
    VulkanCommandBufferBegin(out_command_buffer, true, false, false);
}

//End recording, submits to and waits for queue operation and frees the provided command buffer
void VulkanCommandBufferEndSingleUse(VulkanContext* context, VkCommandPool pool,
                                     VulkanCommandBuffer* command_buffer, VkQueue queue) {
    //End command buffer
    VulkanCommandBufferEnd(command_buffer);

    VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &command_buffer->handle;
    VK_CHECK(vkQueueSubmit(queue, 1, &submitInfo, 0));

    //Wait for it to finish
    VK_CHECK(vkQueueWaitIdle(queue));

    //Free command buffer
    VulkanCommandBufferFree(context, pool, command_buffer);
}