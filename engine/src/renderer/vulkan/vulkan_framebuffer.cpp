#include "vulkan_framebuffer.h"
#include "core/dmemory.h"

void VulkanFrameBufferCreate(
    VulkanContext* context,
    VulkanRenderPass* render_pass,
    u32 width, u32 height, u32 attachment_count,
    VkImageView* attachments,
    VulkanFrameBuffer* out_frame_buffer) {

    //Take a copy of th attachments, renderpass and attachment count
    out_frame_buffer->attachments = (VkImageView*)DAllocate(sizeof(VkImageView) * attachment_count, MEMORY_TAG_RENDERER);
    for (u32 i = 0; i < attachment_count; i++) {
        out_frame_buffer->attachments[i] = attachments[i];
    }
    out_frame_buffer->render_pass = render_pass;
    out_frame_buffer->attachment_count = attachment_count;

    //Creation info
    VkFramebufferCreateInfo frame_buffer_create_info = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    frame_buffer_create_info.renderPass = render_pass->handle;
    frame_buffer_create_info.attachmentCount = attachment_count;
    frame_buffer_create_info.pAttachments = out_frame_buffer->attachments;
    frame_buffer_create_info.width = width;
    frame_buffer_create_info.height = height;
    frame_buffer_create_info.layers = 1;
    VK_CHECK(vkCreateFramebuffer(
        context->device.logical_device,
        &frame_buffer_create_info,
        context->allocator,
        &out_frame_buffer->handle));
}

void VulkanFrameBufferDestroy(VulkanContext* context, VulkanFrameBuffer* frame_buffer) {
    vkDestroyFramebuffer(context->device.logical_device, frame_buffer->handle, context->allocator);
    if (frame_buffer->attachments) {
        DFree(frame_buffer->attachments, sizeof(VkImageView) * frame_buffer->attachment_count, MEMORY_TAG_RENDERER);
        frame_buffer->attachments = 0;
    }
    frame_buffer->handle = 0;
    frame_buffer->attachment_count = 0;
    frame_buffer->render_pass = 0;
}