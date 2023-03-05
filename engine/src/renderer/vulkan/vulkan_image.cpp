#include "vulkan_image.h"
#include "vulkan_device.h"
#include "core/logger.h"
#include "core/dmemory.h"

void VulkanImageCreate(
    VulkanContext* context,
    VkImageType imageType,
    u32 width,
    u32 height,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags memoryFlags,
    b32 createView,
    VkImageAspectFlags viewAspectFlags,
    VulkanImage* outImage){
    
    //copy params
    outImage->width = width;
    outImage->height = height;

    //Creation info
    VkImageCreateInfo imageCreateInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.extent.width = width;
    imageCreateInfo.extent.height = height;
    imageCreateInfo.extent.depth = 1; //TODO: support configurable depth
    imageCreateInfo.mipLevels = 4;    //TODO: support mip mapping
    imageCreateInfo.arrayLayers = 1;  //TODO: support number of layers in the image
    imageCreateInfo.format = format;
    imageCreateInfo.tiling = tiling; //OPTIMAL lets impl decide the best layout
    imageCreateInfo.initialLayout =  VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.usage = usage;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;         //TODO: configrable sample count
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; //TODO: configurable sharing mode

    VK_CHECK(vkCreateImage(context->device.logical_device, &imageCreateInfo, context->allocator, &outImage->handle));

    //get memory requirements
    VkMemoryRequirements memoryRequirements = {};
    vkGetImageMemoryRequirements(context->device.logical_device, outImage->handle, &memoryRequirements);

    i32 memoryType = context->FindMemoryIndex(memoryRequirements.memoryTypeBits, memoryFlags);
    if(memoryType == -1){
        DERROR("Required memory type not found. Image not valid.");
    }

    //Allocate memory
    VkMemoryAllocateInfo memoryAllocateInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = memoryType;
    VK_CHECK(vkAllocateMemory(context->device.logical_device, &memoryAllocateInfo, context->allocator, &outImage->memory));

    //Bind memory
    VK_CHECK(vkBindImageMemory(context->device.logical_device, outImage->handle, outImage->memory, 0)); //TODO: configurable memory offset

    if(createView){
        outImage->view = 0;
        VulkanImageViewCreate(context, format, outImage, viewAspectFlags);
    }
}


void VulkanImageViewCreate(
    VulkanContext* context,
    VkFormat format,
    VulkanImage* image,
    VkImageAspectFlags aspectFlags){
        
    VkImageViewCreateInfo viewCreateInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    viewCreateInfo.image = image->handle;
    viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; //TODO: make configurable
    viewCreateInfo.format = format;
    viewCreateInfo.subresourceRange.aspectMask = aspectFlags;
    //TODO: make configurable
    viewCreateInfo.subresourceRange.baseMipLevel = 0;
    viewCreateInfo.subresourceRange.levelCount = 1;
    viewCreateInfo.subresourceRange.baseArrayLayer = 0;
    viewCreateInfo.subresourceRange.layerCount = 1;

    VK_CHECK(vkCreateImageView(context->device.logical_device, &viewCreateInfo, context->allocator, &image->view));
}

void VulkanImageTransitionLayout(VulkanContext* context, VulkanCommandBuffer* command_buffer, VulkanImage* image, 
                                 VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout) {
    VkImageMemoryBarrier barrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = context->device.graphics_queue_index;
    barrier.dstQueueFamilyIndex = context->device.graphics_queue_index;
    barrier.image = image->handle;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags src_stage;
    VkPipelineStageFlags dst_stage;

    //dont care about the old layout, transition to optimal layout
    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        //dont care what stage the pipeline is in at the start
        src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        //used for copying
        dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        //transitioning from a transfer destination layout to a shader read only layout
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        //from a copying stage to...
        src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        //the fragment stage
        dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        DFATAL("Unsupported layout transition");
        return;
    }

    vkCmdPipelineBarrier(command_buffer->handle, src_stage, dst_stage, 0, 0, 0, 0, 0, 1, &barrier);
}

//copies data in buffer ot provided image
void VulkanImageCopyFromBuffer(VulkanContext* context, VulkanImage* image, VkBuffer buffer, VulkanCommandBuffer* command_buffer) {
    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;    
    region.imageSubresource.mipLevel = 0;    
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageExtent.width = image->width;
    region.imageExtent.height = image->height;
    region.imageExtent.depth = 1;

    vkCmdCopyBufferToImage(command_buffer->handle, buffer, image->handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void VulkanImageDestroy(VulkanContext* context, VulkanImage* image){
    if(image->view){
        vkDestroyImageView(context->device.logical_device, image->view, context->allocator);
        image->view = 0;
    }
    if(image->memory){
        vkFreeMemory(context->device.logical_device, image->memory, context->allocator);
        image->memory = 0;
    }
    if(image->handle){
        vkDestroyImage(context->device.logical_device, image->handle, context->allocator);
        image->handle = 0;
    }
}