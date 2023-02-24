#include "vulkan_device.h"
#include "core/logger.h"
#include "core/dmemory.h"
#include "core/logger.h"
#include "containers/darray.h"
#include "core/dstring.h"

//NOTE: I feel like a lot of the extension and property 
//gathering can be stored in a semi-permanent stack arena

struct VulkanPhysicalDeviceRequirements{
    b8 graphics;
    b8 present;
    b8 compute;
    b8 transfer;
    char** deviceExtensionNames;
    b8 samplerAnisotropy;
    b8 discreteGpu;
};

struct VulkanPhysicalDeviceQueueFamilyInfo{
    u32 graphicsFamilyIndex;
    u32 presentFamilyIndex;
    u32 computeFamilyIndex;
    u32 transferFamilyIndex;
};

b8 SelectPhysicalDevice(VulkanContext* context);
b8 PhysicalDeviceMeetsRequirements(
    VkPhysicalDevice device, 
    VkSurfaceKHR surface,
    VkPhysicalDeviceProperties* properties,
    VkPhysicalDeviceFeatures* features,
    VulkanPhysicalDeviceRequirements* requirements,
    VulkanPhysicalDeviceQueueFamilyInfo * outQueueInfo,
    VulkanSwapchainSupportInfo* outSwapchainInfo);

b8 VulkanDeviceCreate(VulkanContext* context){
    if(!SelectPhysicalDevice(context)){
        return false;
    }

    DINFO("Creating logical device...");
    //NOTE: dont create additional queues for shared indices
    b8 presentSharesGraphicsQueue = context->device.graphicsQueueIndex == context->device.presentQueueIndex;
    b8 transferSharesGraphicsQueue = context->device.graphicsQueueIndex == context->device.transferQueueIndex;
    u32 indexCount = 1;
    if(!presentSharesGraphicsQueue){
        indexCount++;
    }
    if(!transferSharesGraphicsQueue){
        indexCount++;
    }
    u32 indices[32];
    u8 index = 0;
    indices[index++] = context->device.graphicsQueueIndex;
    if(!presentSharesGraphicsQueue){
        indices[index++] = context->device.presentQueueIndex;
    }
    if(!transferSharesGraphicsQueue){
        indices[index++] = context->device.transferQueueIndex;
    }
    VkDeviceQueueCreateInfo queueCreateInfos[32];
    for(u32 i = 0; i < indexCount; i++){
        queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfos[i].queueFamilyIndex = indices[i];
        queueCreateInfos[i].queueCount = 1;
        //enable for future enhancement
        //if(indices[i] == context->device.graphicsQueueIndex){
        //    queueCreateInfos[i].queueCount = 2;
        //}
        queueCreateInfos[i].flags = 0;
        queueCreateInfos[i].pNext = 0;
        f32 queuePriority = 1.0f;
        queueCreateInfos[i].pQueuePriorities = &queuePriority;
    }

    //Request device features
    //TODO: should be config driven
    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo deviceCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    deviceCreateInfo.queueCreateInfoCount = indexCount;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = 1;
    char* extensionNames = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    deviceCreateInfo.ppEnabledExtensionNames = &extensionNames;

    VK_CHECK(vkCreateDevice(
        context->device.physicalDevice,
        &deviceCreateInfo,
        context->allocator,
        &context->device.logicalDevice));

    DINFO("Logical device created.");

    //Get queues
    vkGetDeviceQueue(
        context->device.logicalDevice,
        context->device.graphicsQueueIndex,
        0,
        &context->device.graphicsQueue);
    vkGetDeviceQueue(
        context->device.logicalDevice,
        context->device.presentQueueIndex,
        0,
        &context->device.presentQueue);
    vkGetDeviceQueue(
        context->device.logicalDevice,
        context->device.transferQueueIndex,
        0,
        &context->device.transferQueue);
    DINFO("Queues obtained.");

    //Create command pool for graphis queue
    VkCommandPoolCreateInfo poolCreateInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    poolCreateInfo.queueFamilyIndex = context->device.graphicsQueueIndex;
    poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CHECK(vkCreateCommandPool(
        context->device.logicalDevice,
        &poolCreateInfo,
        context->allocator,
        &context->device.graphicsCommandPool));
    DINFO("Graphics command pool created.");

    return true;
}

void VulkanDeviceDestroy(VulkanContext* context){
    //unset queues
    context->device.graphicsQueue = 0;
    context->device.presentQueue = 0;
    context->device.transferQueue = 0;

    //Destroying command pools
    DINFO("Destroying command pools...");
    vkDestroyCommandPool(context->device.logicalDevice, context->device.graphicsCommandPool, context->allocator);

    DINFO("Destroying logical device...");
    if(context->device.logicalDevice){
        vkDestroyDevice(context->device.logicalDevice, context->allocator);
        context->device.logicalDevice = 0;
    }

    DINFO("Releasing physical device resources...");
    context->device.physicalDevice = 0;

    if(context->device.swapchainSupport.formats){
        DFree(context->device.swapchainSupport.formats,
              sizeof(VkSurfaceFormatKHR) * context->device.swapchainSupport.formatCount,
              MEMORY_TAG_RENDERER);
        context->device.swapchainSupport.formats = 0;
        context->device.swapchainSupport.formatCount = 0;
    }

    if(context->device.swapchainSupport.presentModes){
        DFree(context->device.swapchainSupport.presentModes,
              sizeof(VkSurfaceFormatKHR) * context->device.swapchainSupport.presentModeCount,
              MEMORY_TAG_RENDERER);
        context->device.swapchainSupport.presentModes = 0;
        context->device.swapchainSupport.presentModeCount = 0;
    }
    DZeroMemory(&context->device.swapchainSupport.capabilities, sizeof(context->device.swapchainSupport.capabilities));
    context->device.graphicsQueueIndex = -1;
    context->device.presentQueueIndex = -1;
    context->device.transferQueueIndex = -1;
}

void VulkanDeviceQuerySwapchainSupport(
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    VulkanSwapchainSupportInfo* outSupportInfo){
        VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &outSupportInfo->capabilities));
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &outSupportInfo->formatCount, 0));
        if(outSupportInfo->formatCount != 0){
            if(!outSupportInfo->formats){
                outSupportInfo->formats = (VkSurfaceFormatKHR*)DAllocate(sizeof(VkSurfaceFormatKHR) * outSupportInfo->formatCount, MEMORY_TAG_RENDERER);
            }
            VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &outSupportInfo->formatCount, outSupportInfo->formats));

        }

        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &outSupportInfo->presentModeCount, 0));
        if(outSupportInfo->presentModeCount != 0){
            if(!outSupportInfo->presentModes){
                outSupportInfo->presentModes = (VkPresentModeKHR*)DAllocate(sizeof(VkPresentModeKHR) * outSupportInfo->presentModeCount, MEMORY_TAG_RENDERER);
            }
            VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &outSupportInfo->presentModeCount, outSupportInfo->presentModes));
        }
}

b8 VulkanDeviceDetectDepthFormat(VulkanDevice* device){
    //format candidates
    u64 candidateCount = 3;
    VkFormat candidates[3] = {
        VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT
    };
    u32 flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT; //using depth and stencil buffers
    for(u32 i = 0; i < candidateCount; i++){
        VkFormatProperties properties = {};
        vkGetPhysicalDeviceFormatProperties(device->physicalDevice, candidates[i], &properties);
        if((properties.linearTilingFeatures & flags) == flags){
            device->depthFormat = candidates[i];
            return true;
        } else if((properties.optimalTilingFeatures & flags) == flags){
            device->depthFormat = candidates[i];
            return true;
        }
    }
    return false;
}

b8 SelectPhysicalDevice(VulkanContext* context){
    u32 physicalDeviceCount = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(context->instance, &physicalDeviceCount, 0));
    if(physicalDeviceCount == 0){
        DFATAL("No devices which support Vulkan were found");
        return false;
    }

    VkPhysicalDevice physicalDevices[32];
    VK_CHECK(vkEnumeratePhysicalDevices(context->instance, &physicalDeviceCount, physicalDevices));
    for(u32 i = 0; i < physicalDeviceCount; i++){
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceFeatures features;
        VkPhysicalDeviceMemoryProperties memory;

        vkGetPhysicalDeviceProperties(physicalDevices[i], &properties);
        vkGetPhysicalDeviceFeatures(physicalDevices[i], &features);
        vkGetPhysicalDeviceMemoryProperties(physicalDevices[i], &memory);

        VulkanPhysicalDeviceRequirements requirements = {};
        requirements.graphics = true;
        requirements.present = true;
        requirements.transfer = true;
        //Enable if compute is necessary
        //requirements.compute = true;
        requirements.samplerAnisotropy = true;
        requirements.discreteGpu = true;
        requirements.deviceExtensionNames = (char**)DarrayCreate(const char*);
        DarrayPush(requirements.deviceExtensionNames, (char*)VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        VulkanPhysicalDeviceQueueFamilyInfo queueInfo = {};
        b8 result = PhysicalDeviceMeetsRequirements(physicalDevices[i], context->surface,
                                                    &properties, &features, &requirements, 
                                                    &queueInfo, &context->device.swapchainSupport);

        if(result){
            DINFO("Selected device: '%s'.", properties.deviceName);
            switch(properties.deviceType){
                default:
                case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                    DINFO("GPU type is Unknown.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                    DINFO("GPU type is Integrated");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                    DINFO("GPU type is Discrete");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                    DINFO("GPU type is Virtual");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_CPU:
                    DINFO("GPU type is CPU");
                    break;
            }
            DINFO("GPU Driver Version: %d.%d.%d",
                  VK_VERSION_MAJOR(properties.driverVersion),
                  VK_VERSION_MINOR(properties.driverVersion), 
                  VK_VERSION_PATCH(properties.driverVersion));

            DINFO("Vulkan API Version: %d.%d.%d",
                  VK_VERSION_MAJOR(properties.apiVersion),
                  VK_VERSION_MINOR(properties.apiVersion), 
                  VK_VERSION_PATCH(properties.apiVersion));
            
            for(u32 j = 0; j < memory.memoryHeapCount; j++){
                f32 memorySizeGib = (((f32)memory.memoryHeaps[j].size) / 1024.0f / 1024.0f / 1024.0f);
                if(memory.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT){
                    DINFO("Local GPU memory: %.2f GiB", memorySizeGib);
                } else{
                    DINFO("Shared System memory: %.2f GiB", memorySizeGib);
                }
            }

            context->device.physicalDevice = physicalDevices[i];
            context->device.graphicsQueueIndex = queueInfo.graphicsFamilyIndex;
            context->device.presentQueueIndex = queueInfo.presentFamilyIndex;
            context->device.transferQueueIndex = queueInfo.transferFamilyIndex;
            //NOTE: set compute index here
            context->device.properties = properties;
            context->device.features = features;
            context->device.memory = memory;
            break;
        }
    }
    if(!context->device.physicalDevice){
        DERROR("No physical devices were found which meet the requirements.");
        return false;
    }
    DINFO("Physical device selected.");
    return true;
}

b8 PhysicalDeviceMeetsRequirements(VkPhysicalDevice device, 
                                   VkSurfaceKHR surface,
                                   VkPhysicalDeviceProperties* properties,
                                   VkPhysicalDeviceFeatures* features,
                                   VulkanPhysicalDeviceRequirements* requirements,
                                   VulkanPhysicalDeviceQueueFamilyInfo * outQueueInfo,
                                   VulkanSwapchainSupportInfo* outSwapchainInfo){

    outQueueInfo->graphicsFamilyIndex = -1;
    outQueueInfo->presentFamilyIndex = -1;
    outQueueInfo->computeFamilyIndex = -1;
    outQueueInfo->transferFamilyIndex = -1;
    
    if(requirements->discreteGpu){
        if(properties->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
            DINFO("Device is not a discrete GPU, and one is required. Skipping.")
            return false;
        }
    }

    u32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, 0);
    VkQueueFamilyProperties queueFamilies[32];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

    DINFO("Graphics | Present | Compute | Transfer | Name");
    u8 minTransferScore = 255;
    for(u32 i = 0; i < queueFamilyCount; i++){
        u8 currentTransferScore = 0;

        //Graphics queue?
        if(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT){
            outQueueInfo->graphicsFamilyIndex = i;
            currentTransferScore++;
        }

        //Compute queue?
        if(queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT){
            outQueueInfo->computeFamilyIndex = i;
            currentTransferScore++;
        }

        //Transfer queue?
        if(queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT){
            //Take the index if it is the current lowest.
            //This increase the liklihood that it is a dedicated transfer queue.
            if(currentTransferScore <= minTransferScore){
                minTransferScore = currentTransferScore;
                outQueueInfo->transferFamilyIndex = i;
            }
        }

        //Present queue?
        VkBool32 supportsPresent = VK_FALSE;
        VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supportsPresent));
        if(supportsPresent){
            outQueueInfo->presentFamilyIndex = i;
        }
    }

    DINFO("       %d |       %d |       %d |       %d | %s",
        outQueueInfo->graphicsFamilyIndex != -1,
        outQueueInfo->presentFamilyIndex != -1,
        outQueueInfo->computeFamilyIndex != -1,
        outQueueInfo->transferFamilyIndex != -1,
        properties->deviceName);

    if((!requirements->graphics || (requirements->graphics && outQueueInfo->graphicsFamilyIndex != -1)) &&
        (!requirements->present || (requirements->present && outQueueInfo->presentFamilyIndex != -1)) &&
        (!requirements->compute || (requirements->compute && outQueueInfo->computeFamilyIndex != -1)) &&
        (!requirements->transfer || (requirements->transfer && outQueueInfo->transferFamilyIndex != -1))){

        DINFO("Device meets queue requirements.");
        DTRACE("Graphics Family Index: %i", outQueueInfo->graphicsFamilyIndex); 
        DTRACE("Present Family Index: %i", outQueueInfo->presentFamilyIndex); 
        DTRACE("Transfer Family Index: %i", outQueueInfo->transferFamilyIndex); 
        DTRACE("Compute Family Index: %i", outQueueInfo->computeFamilyIndex); 

        VulkanDeviceQuerySwapchainSupport(device, surface, outSwapchainInfo);
        if(outSwapchainInfo->formatCount < 1 || outSwapchainInfo->presentModeCount < 1){
            if(outSwapchainInfo->formats){
                DFree(outSwapchainInfo->formats, sizeof(VkSurfaceKHR) * outSwapchainInfo->formatCount, MEMORY_TAG_RENDERER);
            }
            if(outSwapchainInfo->presentModes){
                DFree(outSwapchainInfo->presentModes, sizeof(VkPresentModeKHR) * outSwapchainInfo->presentModeCount, MEMORY_TAG_RENDERER);
            }
            DINFO("Required swapchain support not present, skipping device.");
            return false;
        }

        if(requirements->deviceExtensionNames){
            u32 availableExtensionCount = 0;
            VkExtensionProperties* availableExtensions = 0;
            VK_CHECK(vkEnumerateDeviceExtensionProperties(device, 0, &availableExtensionCount, 0));
            if(availableExtensionCount != 0){
                availableExtensions = (VkExtensionProperties*)DAllocate(sizeof(VkExtensionProperties) * availableExtensionCount, MEMORY_TAG_RENDERER);
                VK_CHECK(vkEnumerateDeviceExtensionProperties(device, 0, &availableExtensionCount, availableExtensions));
                u32 requiredExtensionCount = DarrayLength(requirements->deviceExtensionNames);
                for(u32 i = 0; i < requiredExtensionCount; i++){
                    b8 found = false;
                    for(u32 j= 0; j < availableExtensionCount; j++){
                        if(StringsEqual(requirements->deviceExtensionNames[i], availableExtensions[j].extensionName)){
                            found = true;
                            break;
                        }
                    }
                    if(!found){
                        DINFO("Required extension not found: '%s', skipping device.", requirements->deviceExtensionNames[i]);
                        DFree(availableExtensions, sizeof(VkExtensionProperties) * availableExtensionCount, MEMORY_TAG_RENDERER);
                        return false;
                    }
                }
            }
            DFree(availableExtensions, sizeof(VkExtensionProperties) * availableExtensionCount, MEMORY_TAG_RENDERER);
        }

        if(requirements->samplerAnisotropy && !features->samplerAnisotropy){
            DINFO("Device does not support samplerAnisotropy, skipping.");
            return false;
        }

        //Device meets all requirements
        return true;
    }
    return false;
}