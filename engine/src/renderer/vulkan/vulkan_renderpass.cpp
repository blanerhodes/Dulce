#include "vulkan_renderpass.h"
#include "core/dmemory.h"

void VulkanRenderPassCreate(
    VulkanContext* context,
    VulkanRenderPass* outRenderPass,
    f32 x, f32 y, f32 w, f32 h,
    f32 r, f32 g, f32 b, f32 a,
    f32 depth, u32 stencil){

    outRenderPass->x = x;
    outRenderPass->y = y;
    outRenderPass->w = w;
    outRenderPass->h = h;

    outRenderPass->r = r;
    outRenderPass->b = b;
    outRenderPass->g = g;
    outRenderPass->a = a;
    
    outRenderPass->depth = depth;
    outRenderPass->stencil = stencil;

    //Main subpass
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    //Attachemnts TODO: make this configurable
    u32 attachmentDescriptionCount = 2;
    VkAttachmentDescription attachmentDescriptions[2];

    //Color attachment
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = context->swapchain.image_format.format; //TODO: make configurable   
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; //Do not expect any particalr layout before render pass starts
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; //Transitioned to after the render pass
    colorAttachment.flags = 0;

    attachmentDescriptions[0] = colorAttachment;

    VkAttachmentReference colorAttachmentReference = {};
    colorAttachmentReference.attachment = 0; //Attachment description array index
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentReference;

    //Depth attachment, if there is one
    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = context->device.depth_format;        
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    attachmentDescriptions[1] = depthAttachment;

    //Depth attachment
    VkAttachmentReference depthAttachmentReference = {};
    depthAttachmentReference.attachment = 1;
    depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    //TODO: other attachment types (input, resolve, preserve)

    //Depht stencil data
    subpass.pDepthStencilAttachment = &depthAttachmentReference;

    //Input from a shader
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = 0;

    //Attachments used for multisampling color attachments
    subpass.pResolveAttachments = 0;

    //Attachments not used in this subpass, but must be preserved for the next
    subpass.preserveAttachmentCount = 0;
    subpass.pResolveAttachments = 0;

    //Render pass dependencies TODO: make this configurable
    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dependencyFlags = 0;

    //Renderpass create
    VkRenderPassCreateInfo renderpassCreateInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    renderpassCreateInfo.attachmentCount = attachmentDescriptionCount;    
    renderpassCreateInfo.pAttachments = attachmentDescriptions;
    renderpassCreateInfo.subpassCount = 1;
    renderpassCreateInfo.pSubpasses = &subpass;
    renderpassCreateInfo.dependencyCount = 1;
    renderpassCreateInfo.pDependencies = &dependency;
    renderpassCreateInfo.pNext = 0;
    renderpassCreateInfo.flags = 0;

    VK_CHECK(vkCreateRenderPass(
        context->device.logical_device,
        &renderpassCreateInfo,
        context->allocator,
        &outRenderPass->handle));
}

void VulkanRenderPassDestroy(VulkanContext* context, VulkanRenderPass* renderPass){
    if(renderPass && renderPass->handle){
        vkDestroyRenderPass(context->device.logical_device, renderPass->handle, context->allocator);
        renderPass->handle = 0;
    }
}

void VulkanRenderPassBegin(
    VulkanCommandBuffer* commandBuffer,
    VulkanRenderPass* renderPass,
    VkFramebuffer frameBuffer){
    
    VkRenderPassBeginInfo beginInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    beginInfo.renderPass = renderPass->handle;
    beginInfo.framebuffer = frameBuffer;
    beginInfo.renderArea.offset.x = renderPass->x;
    beginInfo.renderArea.offset.y = renderPass->y;
    beginInfo.renderArea.extent.width = renderPass->w;
    beginInfo.renderArea.extent.height = renderPass->h;

    VkClearValue clearValues[2] = {};
    DZeroMemory(clearValues, sizeof(VkClearValue) * 2);
    clearValues[0].color.float32[0] = renderPass->r;
    clearValues[0].color.float32[1] = renderPass->g;
    clearValues[0].color.float32[2] = renderPass->b;
    clearValues[0].color.float32[3] = renderPass->a;
    clearValues[1].depthStencil.depth = renderPass->depth;
    clearValues[1].depthStencil.stencil = renderPass->stencil;

    beginInfo.clearValueCount = 2;
    beginInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(commandBuffer->handle, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
    commandBuffer->state = COMMAND_BUFFER_STATE_IN_RENDER_PASS;
}

void VulkanRenderPassEnd(VulkanCommandBuffer* commandBuffer, VulkanRenderPass* renderPass){
    vkCmdEndRenderPass(commandBuffer->handle);
    commandBuffer->state = COMMAND_BUFFER_STATE_RECORDING;
}