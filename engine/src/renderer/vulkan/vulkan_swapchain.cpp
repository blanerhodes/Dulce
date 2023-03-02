#include "vulkan_swapchain.h"

#include "core/logger.h"
#include "core/dmemory.h"
#include "vulkan_device.h"
#include "vulkan_image.h"

void Create(VulkanContext* context, u32 width, u32 height, VulkanSwapchain* swapchain);

void Destroy(VulkanContext* context, VulkanSwapchain* swapchain);

void VulkanSwapchainCreate(VulkanContext* context, u32 width, u32 height, VulkanSwapchain* out_swapchain) {
    Create(context, width, height, out_swapchain);
}

void VulkanSwapchainRecreate(VulkanContext* context, u32 width, u32 height, VulkanSwapchain* swapchain) {
    Destroy(context, swapchain);
    Create(context, width, height, swapchain);
}

void VulkanSwapchainDestroy(VulkanContext* context, VulkanSwapchain* swapchain) {
    Destroy(context, swapchain);
}

b8 VulkanSwapchainAcquireNextImageIndex(
    VulkanContext* context, 
    VulkanSwapchain* swapchain,
    u64 timeoutNs,
    VkSemaphore image_available_semaphore,
    VkFence fence,
    u32* out_image_index){
    VkResult result = vkAcquireNextImageKHR(context->device.logical_device,
                                            swapchain->handle, timeoutNs,
                                            image_available_semaphore, fence, out_image_index);                                
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        VulkanSwapchainRecreate(context, context->frame_buffer_width,
                                context->frame_buffer_height, swapchain);
        return false;
    } else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        DFATAL("Failed to acquire swapchain image!");
        return false;
    }
    return true;
}

void VulkanSwapchainPresent(
    VulkanContext* context,
    VulkanSwapchain* swapchain,
    VkQueue graphics_queue,
    VkQueue present_queue,
    VkSemaphore render_complete_semaphore,
    u32 present_image_index){
    //Return the image to the swapchain for presentation
    VkPresentInfoKHR present_info = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    present_info.waitSemaphoreCount = 1;  
    present_info.pWaitSemaphores = &render_complete_semaphore;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &swapchain->handle;
    present_info.pImageIndices = &present_image_index;;
    present_info.pResults = 0;
    
    VkResult result = vkQueuePresentKHR(present_queue, &present_info);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        VulkanSwapchainRecreate(context, context->frame_buffer_width,
                                context->frame_buffer_height, swapchain);
    } else if(result != VK_SUCCESS) {
        DFATAL("Failed to present swap chain image!");
    }

    //Increment and loop the index
    context->current_frame = (context->current_frame + 1) % swapchain->max_frames_in_flight;
}

void Create(VulkanContext* context, u32 width, u32 height, VulkanSwapchain* swapchain) {
    VkExtent2D swapchain_extent = {width, height};

    //choose swap surface format
    b8 found = false;
    for (u32 i = 0; i < context->device.swapchain_support.format_count; i++) {
        VkSurfaceFormatKHR format = context->device.swapchain_support.formats[i];
        //preffered formats
        if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            swapchain->image_format = format;
            found = true;
            break;
        }
    }
    if (!found) {
        swapchain->image_format = context->device.swapchain_support.formats[0];
    }

    //per the spec FIFO is the only one guaranteed to exist
    //FIFO always displays all frames in a proper order even when newer frames exist
    //since you most likely render frames faster than presenting this may not be as responsive
    //**IMMEDIATE is fastest, needs 2 images, but doesnt make app wait for vertical blank
    //**Non vsync and can cause screen tearing 
    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR; 
    for (u32 i = 0; i < context->device.swapchain_support.present_mode_count; i++) {
        VkPresentModeKHR mode = context->device.swapchain_support.present_modes[i];
        //MAILBOX says use most current image available, toss the rest, uses triple buffering
        //lowest latency
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            present_mode = mode;
            break;
        }
    }
    VulkanDeviceQuerySwapchainSupport(context->device.physical_device, context->surface, &context->device.swapchain_support);
    //SwapchainExtent, overrides if what were given is BS
    if (context->device.swapchain_support.capabilities.currentExtent.width != UINT32_MAX) {
        swapchain_extent = context->device.swapchain_support.capabilities.currentExtent;
    }
    //clamp to value allowed by the GPU
    VkExtent2D min = context->device.swapchain_support.capabilities.minImageExtent;
    VkExtent2D max = context->device.swapchain_support.capabilities.maxImageExtent;
    swapchain_extent.width = DCLAMP(swapchain_extent.width, min.width, max.width);
    swapchain_extent.height = DCLAMP(swapchain_extent.height, min.height, max.height);

    u32 image_count = context->device.swapchain_support.capabilities.minImageCount + 1;
    if (context->device.swapchain_support.capabilities.maxImageCount > 0
       && image_count > context->device.swapchain_support.capabilities.maxImageCount) {
        image_count =  context->device.swapchain_support.capabilities.maxImageCount;
    }
    //denotes triple buffering where one is showing and 2 are available to write to
    swapchain->max_frames_in_flight = image_count - 1;

    VkSwapchainCreateInfoKHR swapchain_create_info = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    swapchain_create_info.surface = context->surface;
    swapchain_create_info.minImageCount = image_count;
    swapchain_create_info.imageFormat = swapchain->image_format.format;
    swapchain_create_info.imageColorSpace = swapchain->image_format.colorSpace;
    swapchain_create_info.imageExtent = swapchain_extent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; //color buffer

    //setup the queue family indices
    if (context->device.graphics_queue_index != context->device.present_queue_index) {
        u32 queue_family_indices[] = {(u32)context->device.graphics_queue_index, (u32)context->device.present_queue_index};
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT; //can be used by more than 1 family index at once
        swapchain_create_info.queueFamilyIndexCount = 2;
        swapchain_create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; //only used by 1 family index at a time
        swapchain_create_info.queueFamilyIndexCount = 0;
        swapchain_create_info.pQueueFamilyIndices = 0;
    }

    //preTransform deals with presentation vs image (ex. portrait vs landscape on mobile)
    swapchain_create_info.preTransform = context->device.swapchain_support.capabilities.currentTransform;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //opaque composite with OS
    swapchain_create_info.presentMode = present_mode;
    swapchain_create_info.clipped = VK_TRUE;
    swapchain_create_info.oldSwapchain = 0;

    //the call that does all the work
    VK_CHECK(vkCreateSwapchainKHR(context->device.logical_device, &swapchain_create_info,
                                  context->allocator, &swapchain->handle));
    //in a swapchain, the swapchain creates the images it's going to use so we 
    //have to get them and destroy them ourselves
    //start with a zero frame index
    context->current_frame = 0;
    //Images
    swapchain->image_count = 0;
    VK_CHECK(vkGetSwapchainImagesKHR(context->device.logical_device, swapchain->handle, &swapchain->image_count, 0));
    if (!swapchain->images) {
        swapchain->images = (VkImage*)DAllocate(sizeof(VkImage) * swapchain->image_count, MEMORY_TAG_RENDERER);
    }
    if (!swapchain->views) {
        swapchain->views = (VkImageView*)DAllocate(sizeof(VkImageView) * swapchain->image_count, MEMORY_TAG_RENDERER);
    }
    VK_CHECK(vkGetSwapchainImagesKHR(context->device.logical_device, swapchain->handle, &swapchain->image_count, swapchain->images));

    //swapchain doesnt create views by defualt though so we make them
    for (u32 i = 0; i < swapchain->image_count; i++) {
        VkImageViewCreateInfo view_info = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        view_info.image = swapchain->images[i];
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = swapchain->image_format.format;
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; //color buffer
        view_info.subresourceRange.baseMipLevel = 0;
        view_info.subresourceRange.levelCount = 1;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount = 1;
        VK_CHECK(vkCreateImageView(context->device.logical_device, &view_info, context->allocator, &swapchain->views[i]));
    }

    //Depth resources
    if (!VulkanDeviceDetectDepthFormat(&context->device)) {
        context->device.depth_format = VK_FORMAT_UNDEFINED;
        DFATAL("Failed to find a supported format!");
    }
    //Depth buffer (image) doesnt provide this to us
    VulkanImageCreate(context, VK_IMAGE_TYPE_2D, swapchain_extent.width,
                      swapchain_extent.height, context->device.depth_format,
                      VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true, VK_IMAGE_ASPECT_DEPTH_BIT,
                      &swapchain->depth_attachment);

    DINFO("Swapchain created successfully.");
}

void Destroy(VulkanContext* context, VulkanSwapchain* swapchain) {
    vkDeviceWaitIdle(context->device.logical_device);
    VulkanImageDestroy(context, &swapchain->depth_attachment);

    //Only destroy the views, not the images since those are owned by the swapchain and are
    //destroyed when it is
    for (u32 i = 0; i < swapchain->image_count; i++) {
        vkDestroyImageView(context->device.logical_device, swapchain->views[i], context->allocator);
    }

    vkDestroySwapchainKHR(context->device.logical_device, swapchain->handle, context->allocator);
}