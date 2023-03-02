#include "vulkan_fence.h"
#include "core/logger.h"


void VulkanFenceCreate(VulkanContext* context, b8 createSignaled, VulkanFence* outFence){
    //Make sure to signal the fence if required
    outFence->is_signaled = createSignaled;
    VkFenceCreateInfo fenceCreateInfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    if(outFence->is_signaled){
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    }
    VK_CHECK(vkCreateFence(context->device.logical_device, &fenceCreateInfo, context->allocator, &outFence->handle));
}

void VulkanFenceDestroy(VulkanContext* context, VulkanFence* fence){
    if(fence->handle){
        vkDestroyFence(context->device.logical_device, fence->handle, context->allocator);
        fence->handle = 0;
    }
    fence->is_signaled = false;
}

b8 VulkanFenceWait(VulkanContext* context, VulkanFence* fence, u64 timeoutNs){
    if(!fence->is_signaled){
        VkResult result = vkWaitForFences(context->device.logical_device, 1, &fence->handle, true, timeoutNs);

        switch(result){
            case VK_SUCCESS:
                fence->is_signaled = true;
                return true;
            case VK_TIMEOUT:
                DWARN("vkFenceWait - Timed out");
                break;
            case VK_ERROR_DEVICE_LOST:
                DERROR("vkFenceWait - VK_ERROR_DEVICE_LOST")
                break;
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                DERROR("vkFenceWait - VK_ERROR_OUT_OF_HOST_MEMORY")
                break;
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                DERROR("vkFenceWait - VK_ERROR_OUT_OF_DEVICE_MEMORY")
                break;
            default:
                DERROR("vkFenceWait - An unknown error has occurred.")
                break;
        }
    } else{
        return true;
    }
    return false;
}

void VulkanFenceReset(VulkanContext* context, VulkanFence* fence){
    if(fence->is_signaled){
        VK_CHECK(vkResetFences(context->device.logical_device, 1, &fence->handle));
        fence->is_signaled = false;
    }
}