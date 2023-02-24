#include "vulkan_framebuffer.h"
#include "core/dmemory.h"

void VulkanFrameBufferCreate(
    VulkanContext* context,
    VulkanRenderPass* renderPass,
    u32 width, u32 height, u32 attachmentCount,
    VkImageView* attachments,
    VulkanFrameBuffer* outFrameBuffer){

    //Take a copy of th attachments, renderpass and attachment count
    outFrameBuffer->attachments = (VkImageView*)DAllocate(sizeof(VkImageView) * attachmentCount, MEMORY_TAG_RENDERER);
    for(u32 i = 0; i < attachmentCount; i++){
        outFrameBuffer->attachments[i] = attachments[i];
    }
    outFrameBuffer->renderPass = renderPass;
    outFrameBuffer->attachmentCount = attachmentCount;

    //Creation info
    VkFramebufferCreateInfo frameBufferCreateInfo = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    frameBufferCreateInfo.renderPass = renderPass->handle;
    frameBufferCreateInfo.attachmentCount = attachmentCount;
    frameBufferCreateInfo.pAttachments = outFrameBuffer->attachments;
    frameBufferCreateInfo.width = width;
    frameBufferCreateInfo.height = height;
    frameBufferCreateInfo.layers = 1;
    VK_CHECK(vkCreateFramebuffer(
        context->device.logicalDevice,
        &frameBufferCreateInfo,
        context->allocator,
        &outFrameBuffer->handle));
}

void VulkanFrameBufferDestroy(VulkanContext* context, VulkanFrameBuffer* frameBuffer){
    vkDestroyFramebuffer(context->device.logicalDevice, frameBuffer->handle, context->allocator);
    if(frameBuffer->attachments){
        DFree(frameBuffer->attachments, sizeof(VkImageView) * frameBuffer->attachmentCount, MEMORY_TAG_RENDERER);
        frameBuffer->attachments = 0;
    }
    frameBuffer->handle = 0;
    frameBuffer->attachmentCount = 0;
    frameBuffer->renderPass = 0;
}