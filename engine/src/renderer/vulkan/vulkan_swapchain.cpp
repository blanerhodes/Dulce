#include "vulkan_swapchain.h"

#include "core/logger.h"
#include "core/dmemory.h"
#include "vulkan_device.h"
#include "vulkan_image.h"

void Create(VulkanContext* context, u32 width, u32 height, VulkanSwapchain* swapchain);

void Destroy(VulkanContext* context, VulkanSwapchain* swapchain);

void VulkanSwapchainCreate(VulkanContext* context, u32 width, u32 height, VulkanSwapchain* outSwapchain){
    Create(context, width, height, outSwapchain);
}

void VulkanSwapchainRecreate(VulkanContext* context, u32 width, u32 height, VulkanSwapchain* swapchain){
    Destroy(context, swapchain);
    Create(context, width, height, swapchain);
}

void VulkanSwapchainDestroy(VulkanContext* context, VulkanSwapchain* swapchain){
    Destroy(context, swapchain);
}

b8 VulkanSwapchainAcquireNextImageIndex(
    VulkanContext* context, 
    VulkanSwapchain* swapchain,
    u64 timeoutNs,
    VkSemaphore imageAvailableSemaphore,
    VkFence fence,
    u32* outImageIndex){
    VkResult result = vkAcquireNextImageKHR(context->device.logicalDevice,
                                            swapchain->handle, timeoutNs,
                                            imageAvailableSemaphore, fence, outImageIndex);                                
    if(result == VK_ERROR_OUT_OF_DATE_KHR){
        VulkanSwapchainRecreate(context, context->frameBufferWidth,
                                context->frameBufferHeight, swapchain);
        return false;
    } else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
        DFATAL("Failed to acquire swapchain image!");
        return false;
    }
    return true;
}

void VulkanSwapchainPresent(
    VulkanContext* context,
    VulkanSwapchain* swapchain,
    VkQueue graphicsQueue,
    VkQueue presentQueue,
    VkSemaphore renderCompleteSemaphore,
    u32 presentImageIndex){
    //Return the image to the swapchain for presentation
    VkPresentInfoKHR presentInfo = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    presentInfo.waitSemaphoreCount = 1;  
    presentInfo.pWaitSemaphores = &renderCompleteSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain->handle;
    presentInfo.pImageIndices = &presentImageIndex;;
    presentInfo.pResults = 0;
    
    VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);
    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR){
        VulkanSwapchainRecreate(context, context->frameBufferWidth,
                                context->frameBufferHeight, swapchain);
    } else if(result != VK_SUCCESS){
        DFATAL("Failed to present swap chain image!");
    }

    //Increment and loop the index
    context->currentFrame = (context->currentFrame + 1) % swapchain->maxFramesInFlight;
}

void Create(VulkanContext* context, u32 width, u32 height, VulkanSwapchain* swapchain){
    VkExtent2D swapchainExtent = {width, height};

    //choose swap surface format
    b8 found = false;
    for(u32 i = 0; i < context->device.swapchainSupport.formatCount; i++){
        VkSurfaceFormatKHR format = context->device.swapchainSupport.formats[i];
        //preffered formats
        if(format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
            swapchain->imageFormat = format;
            found = true;
            break;
        }
    }
    if(!found){
        swapchain->imageFormat = context->device.swapchainSupport.formats[0];
    }

    //per the spec FIFO is the only one guaranteed to exist
    //FIFO always displays all frames in a proper order even when newer frames exist
    //since you most likely render frames faster than presenting this may not be as responsive
    //**IMMEDIATE is fastest, needs 2 images, but doesnt make app wait for vertical blank
    //**Non vsync and can cause screen tearing 
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR; 
    for(u32 i = 0; i < context->device.swapchainSupport.presentModeCount; i++){
        VkPresentModeKHR mode = context->device.swapchainSupport.presentModes[i];
        //MAILBOX says use most current image available, toss the rest, uses triple buffering
        //lowest latency
        if(mode == VK_PRESENT_MODE_MAILBOX_KHR){
            presentMode = mode;
            break;
        }
    }
    VulkanDeviceQuerySwapchainSupport(context->device.physicalDevice, context->surface, &context->device.swapchainSupport);
    //SwapchainExtent, overrides if what were given is BS
    if(context->device.swapchainSupport.capabilities.currentExtent.width != UINT32_MAX){
        swapchainExtent = context->device.swapchainSupport.capabilities.currentExtent;
    }
    //clamp to value allowed by the GPU
    VkExtent2D min = context->device.swapchainSupport.capabilities.minImageExtent;
    VkExtent2D max = context->device.swapchainSupport.capabilities.maxImageExtent;
    swapchainExtent.width = DCLAMP(swapchainExtent.width, min.width, max.width);
    swapchainExtent.height = DCLAMP(swapchainExtent.height, min.height, max.height);

    u32 imageCount = context->device.swapchainSupport.capabilities.minImageCount + 1;
    if(context->device.swapchainSupport.capabilities.maxImageCount > 0
       && imageCount > context->device.swapchainSupport.capabilities.maxImageCount){
        imageCount =  context->device.swapchainSupport.capabilities.maxImageCount;
    }
    //denotes triple buffering where one is showing and 2 are available to write to
    swapchain->maxFramesInFlight = imageCount - 1;

    VkSwapchainCreateInfoKHR swapchainCreateInfo = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    swapchainCreateInfo.surface = context->surface;
    swapchainCreateInfo.minImageCount = imageCount;
    swapchainCreateInfo.imageFormat = swapchain->imageFormat.format;
    swapchainCreateInfo.imageColorSpace = swapchain->imageFormat.colorSpace;
    swapchainCreateInfo.imageExtent = swapchainExtent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; //color buffer

    //setup the queue family indices
    if(context->device.graphicsQueueIndex != context->device.presentQueueIndex){
        u32 queueFamilyIndices[] {(u32)context->device.graphicsQueueIndex, (u32)context->device.presentQueueIndex};
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; //can be used by more than 1 family index at once
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo. pQueueFamilyIndices = queueFamilyIndices;
    } else{
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; //only used by 1 family index at a time
        swapchainCreateInfo.queueFamilyIndexCount = 0;
        swapchainCreateInfo. pQueueFamilyIndices = 0;
    }

    //preTransform deals with presentation vs image (ex. portrait vs landscape on mobile)
    swapchainCreateInfo.preTransform = context->device.swapchainSupport.capabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //opaque composite with OS
    swapchainCreateInfo.presentMode = presentMode;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = 0;

    //the call that does all the work
    VK_CHECK(vkCreateSwapchainKHR(context->device.logicalDevice, &swapchainCreateInfo,
                                  context->allocator, &swapchain->handle));
    //in a swapchain, the swapchain creates the images it's going to use so we 
    //have to get them and destroy them ourselves
    //start with a zero frame index
    context->currentFrame = 0;
    //Images
    swapchain->imageCount = 0;
    VK_CHECK(vkGetSwapchainImagesKHR(context->device.logicalDevice, swapchain->handle, &swapchain->imageCount, 0));
    if(!swapchain->images){
        swapchain->images = (VkImage*)DAllocate(sizeof(VkImage) * swapchain->imageCount, MEMORY_TAG_RENDERER);
    }
    if(!swapchain->views){
        swapchain->views = (VkImageView*)DAllocate(sizeof(VkImageView) * swapchain->imageCount, MEMORY_TAG_RENDERER);
    }
    VK_CHECK(vkGetSwapchainImagesKHR(context->device.logicalDevice, swapchain->handle, &swapchain->imageCount, swapchain->images));

    //swapchain doesnt create views by defualt though so we make them
    for(u32 i = 0; i < swapchain->imageCount; i++){
        VkImageViewCreateInfo viewInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        viewInfo.image = swapchain->images[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = swapchain->imageFormat.format;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; //color buffer
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;
        VK_CHECK(vkCreateImageView(context->device.logicalDevice, &viewInfo, context->allocator, &swapchain->views[i]));
    }

    //Depth resources
    if(!VulkanDeviceDetectDepthFormat(&context->device)){
        context->device.depthFormat = VK_FORMAT_UNDEFINED;
        DFATAL("Failed to find a supported format!");
    }
    //Depth buffer (image) doesnt provide this to us
    VulkanImageCreate(context, VK_IMAGE_TYPE_2D, swapchainExtent.width,
                      swapchainExtent.height, context->device.depthFormat,
                      VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true, VK_IMAGE_ASPECT_DEPTH_BIT,
                      &swapchain->depthAttachment);

    DINFO("Swapchain created successfully.");
}

void Destroy(VulkanContext* context, VulkanSwapchain* swapchain){
    vkDeviceWaitIdle(context->device.logicalDevice);
    VulkanImageDestroy(context, &swapchain->depthAttachment);

    //Only destroy the views, not the images since those are owned by the swapchain and are
    //destroyed when it is
    for(u32 i = 0; i < swapchain->imageCount; i++){
        vkDestroyImageView(context->device.logicalDevice, swapchain->views[i], context->allocator);
    }

    vkDestroySwapchainKHR(context->device.logicalDevice, swapchain->handle, context->allocator);
}