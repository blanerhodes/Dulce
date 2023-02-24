#include "vulkan_backend.h"
#include "vulkan_types.inl"
#include "vulkan_platform.h"
#include "vulkan_device.h"
#include "vulkan_swapchain.h"
#include "vulkan_renderpass.h"
#include "vulkan_command_buffer.h"
#include "vulkan_framebuffer.h"
#include "vulkan_fence.h"
#include "vulkan_utils.h"

#include "core/logger.h"
#include "core/dstring.h"
#include "containers/darray.h"
#include "core/dmemory.h"

#include "core/application.h"

static VulkanContext context;
static u32 cachedFrameBufferWidth;
static u32 cachedFrameBufferHeight;

VKAPI_ATTR VkBool32 VKAPI_CALL vkDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
    void*  userData);

i32 FindMemoryIndex(u32 typeFilter, u32 propertyFlags);

void CreateCommandBuffers(RendererBackend* backend);
void RegenerateFrameBuffers(RendererBackend* backend, VulkanSwapchain* swapchain, VulkanRenderPass* renderPass);
b8 RecreateSwapchain(RendererBackend* backend);

b8 VulkanRendererBackendInitialize(RendererBackend* backend, char* applicationName, PlatformState* platState){
    //Function pointers
    context.FindMemoryIndex = FindMemoryIndex;

    //TODO: custom allocator
    context.allocator = 0;

    ApplicationGetFrameBufferSize(&cachedFrameBufferWidth, &cachedFrameBufferHeight);
    context.frameBufferWidth = (cachedFrameBufferWidth != 0) ? cachedFrameBufferWidth : 1280;
    context.frameBufferHeight = (cachedFrameBufferHeight != 0) ? cachedFrameBufferHeight : 720;
    cachedFrameBufferWidth = 0;
    cachedFrameBufferHeight = 0;

    VkApplicationInfo appInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    appInfo.apiVersion = VK_API_VERSION_1_3;
    appInfo.pApplicationName = applicationName;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Dulce Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

    VkInstanceCreateInfo createInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    createInfo.pApplicationInfo = &appInfo;

    char** requiredExtensions = (char**)DarrayCreate(char*);
    DarrayPush(requiredExtensions, (char*)VK_KHR_SURFACE_EXTENSION_NAME);
    PlatformGetRequiredExtensionNames(&requiredExtensions);

#if defined(_DEBUG)
    DarrayPush(requiredExtensions, (char*)VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    DDEBUG("Required extensions:");
    u32 length = DarrayLength(requiredExtensions);
    for(u32 i = 0; i < length; i++){
        DDEBUG(requiredExtensions[i]);
    }
#endif

    createInfo.enabledExtensionCount = DarrayLength(requiredExtensions);
    createInfo.ppEnabledExtensionNames = requiredExtensions;

    char** requiredValidationLayerNames = 0;
    u32 requiredValidationLayerCount = 0;

//validation layers
#if defined(_DEBUG)
    DINFO("Validation layers enabled. Enumerating...");

    requiredValidationLayerNames = (char**)DarrayCreate(char*);
    DarrayPush(requiredValidationLayerNames, (char*)"VK_LAYER_KHRONOS_validation");
    requiredValidationLayerCount = DarrayLength(requiredValidationLayerNames);

    u32 availableLayerCount = 0;
    VK_CHECK(vkEnumerateInstanceLayerProperties(&availableLayerCount, 0));
    VkLayerProperties* availableLayers = (VkLayerProperties*)DarrayReserve(VkLayerProperties, availableLayerCount);
    VK_CHECK(vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers));

    for(u32 i = 0; i < requiredValidationLayerCount; i++){
        DINFO("Searching for layer: %s...", requiredValidationLayerNames[i]);
        b8 found = false;
        for(u32 j = 0; j < availableLayerCount; j++){
            if(StringsEqual(requiredValidationLayerNames[i], availableLayers[j].layerName)){
                found = true;
                DINFO("Found.");
                break;
            }
        }
        if(!found){
            DFATAL("Required validation layer is missing: %s", requiredValidationLayerNames[i]);
            return false;
        }
        DINFO("All required validation layers are present");
    }
#endif

    createInfo. enabledLayerCount = requiredValidationLayerCount;
    createInfo.ppEnabledLayerNames = requiredValidationLayerNames;
    
    VK_CHECK(vkCreateInstance(&createInfo, context.allocator, &context.instance));
    DINFO("Vulkan Instance created.");

    //Debugger init
#if defined(_DEBUG)
    DDEBUG("Creating Vulkan debugger...");
    u32 logSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
        // | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    debugCreateInfo.messageSeverity = logSeverity;
    debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debugCreateInfo.pfnUserCallback = vkDebugCallback;

    //gotta do this for access to extension functions
    PFN_vkCreateDebugUtilsMessengerEXT createVkDebugger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, "vkCreateDebugUtilsMessengerEXT");
    DASSERT_MSG(createVkDebugger, "Failed to create debug messenger!");
    VK_CHECK(createVkDebugger(context.instance, &debugCreateInfo, context.allocator, &context.debugMessenger));
    DDEBUG("Vulkan debugger created.");
#endif

    //Surface
    DDEBUG("Creating Vulkan surface...");
    if(!PlatformCreateVulkanSurface(platState, &context)){
        DERROR("Failed to create platform surface!");
        return false;
    }
    DDEBUG("Vulkan surface created.");

    //Device creation
    if(!VulkanDeviceCreate(&context)){
        DERROR("Failed to create device!");
        return false;
    }

    //Swapchain
    VulkanSwapchainCreate(&context, context.frameBufferWidth, context.frameBufferHeight, &context.swapchain);
    DINFO("Vulkan swapchain created.");

    //Renderpass
    VulkanRenderPassCreate(
        &context,
        &context.mainRenderPass,
        0, 0, context.frameBufferWidth, context.frameBufferHeight,
        0.0f, 0.0f, 0.2f, 1.0f,
        1.0f, 0);

    //Swapchain framebuffers
    context.swapchain.frameBuffers = (VulkanFrameBuffer*)DarrayReserve(VulkanFrameBuffer, context.swapchain.imageCount);
    RegenerateFrameBuffers(backend, &context.swapchain, &context.mainRenderPass);

    //Create command buffers
    CreateCommandBuffers(backend);

    //Create sync objects
    context.imageAvailableSemaphores = (VkSemaphore*)DarrayReserve(VkSemaphore, context.swapchain.maxFramesInFlight);
    context.queueCompleteSemaphores = (VkSemaphore*)DarrayReserve(VkSemaphore, context.swapchain.maxFramesInFlight);
    context.inFlightFences = (VulkanFence*)DarrayReserve(VulkanFence, context.swapchain.maxFramesInFlight);

    for(u8 i = 0; i < context.swapchain.maxFramesInFlight; i++){
        VkSemaphoreCreateInfo semaphoreCreateInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        vkCreateSemaphore(context.device.logicalDevice, &semaphoreCreateInfo,
            context.allocator, &context.imageAvailableSemaphores[i]);
        vkCreateSemaphore(context.device.logicalDevice, &semaphoreCreateInfo,
            context.allocator, &context.queueCompleteSemaphores[i]);

        //Create the fence in a signaled state, indicating that the first frame ahs already been "rendered".
        //This will prevent the application form waiting indefinitely for the first frame to render since it
        //cannot be rendered until a frame is "rendered" before it.
        VulkanFenceCreate(&context, true, &context.inFlightFences[i]);
    }

    //In flight fences should nto yet exist at this point, so clera the list. These are stored in pointers
    //because the intial state should be 0, and will be 0 when not in use. Actual fences are not owned
    //by this list.
    context.imagesInFlight = (VulkanFence**)DarrayReserve(VulkanFence, context.swapchain.imageCount);
    for(u32 i = 0; i < context.swapchain.imageCount; i++){
        context.imagesInFlight[i] = 0;
    }

    DINFO("Vulkan renderer initialized successfully.");
    return true;
}

void VulkanRendererBackendShutdown(RendererBackend* backend){
    vkDeviceWaitIdle(context.device.logicalDevice);
    //destroy in opposite order of creation

    //sync objects
    for(u8 i = 0; i < context.swapchain.maxFramesInFlight; i++){
        if(context.imageAvailableSemaphores[i]){
            vkDestroySemaphore(
                context.device.logicalDevice,
                context.imageAvailableSemaphores[i],
                context.allocator);
            context.imageAvailableSemaphores[i] = 0;
        }
        if(context.queueCompleteSemaphores[i]){
            vkDestroySemaphore(
                context.device.logicalDevice,
                context.queueCompleteSemaphores[i],
                context.allocator);
            context.queueCompleteSemaphores[i] = 0;
        }
        VulkanFenceDestroy(&context, &context.inFlightFences[i]);
    }
    DarrayDestroy(context.imageAvailableSemaphores);
    context.imageAvailableSemaphores = 0;
    DarrayDestroy(context.queueCompleteSemaphores);
    context.queueCompleteSemaphores = 0;
    DarrayDestroy(context.inFlightFences);
    context.inFlightFences = 0;

    //Command buffers
    for(u32 i = 0; i < context.swapchain.imageCount; i++){
        if(context.graphicsCommandBuffers[i].handle){
            VulkanCommandBufferFree(&context, context.device.graphicsCommandPool, &context.graphicsCommandBuffers[i]);
            context.graphicsCommandBuffers[i].handle = 0;
        }
    }
    DarrayDestroy(context.graphicsCommandBuffers);
    context.graphicsCommandBuffers = 0;

    //Destroy framebuffers
    for(u32 i = 0; i < context.swapchain.imageCount; i++){
        VulkanFrameBufferDestroy(&context, &context.swapchain.frameBuffers[i]);
    }

    //Renderpass
    VulkanRenderPassDestroy(&context, &context.mainRenderPass);

    //Swapchain
    VulkanSwapchainDestroy(&context, &context.swapchain);

    DDEBUG("Destroying Vulkan device...");
    VulkanDeviceDestroy(&context);

    DDEBUG("Destroying Vulkan surface...");
    if(context.surface){
        vkDestroySurfaceKHR(context.instance, context.surface, context.allocator);
        context.surface = 0;
    }

    DDEBUG("Destroying Vulkan debugger...");
    if(context.debugMessenger){
        PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, "vkDestroyDebugUtilsMessengerEXT");
        func(context.instance, context.debugMessenger, context.allocator);
    }
    DDEBUG("Destroying Vulkan instance...");
    vkDestroyInstance(context.instance, context.allocator);
}

void VulkanRendererBackendOnResized(RendererBackend* backend, u16 width, u16 height){
    //Update the framebuffer size generation, a ocunter which indicates when the framebuffer size has been updated
    cachedFrameBufferWidth = width;
    cachedFrameBufferHeight = height;
    //detect when this counter changes to tell us we received a resize event
    //and need to resize the framebuffer before rendering again
    //after this is complete we sync it with another version and when they're synced it will
    //pass a test to say its updated to the current size and move on
    context.frameBufferSizeGeneration++;
    DINFO("Vulkan renderer backend->resized: w/h/gen: %i/%i/%llu", width, height, context.frameBufferSizeGeneration);
}

b8 VulkanRendererBackendBeginFrame(RendererBackend* backend, f32 deltaTime){
    VulkanDevice* device = &context.device;

    //Check if recrating swap chain and boot out
    if(context.recreatingSwapchain){
        VkResult result = vkDeviceWaitIdle(device->logicalDevice);
        if(!VulkanResultIsSuccess(result)){
            DERROR("VulkanRendererBackendBeginFrame vkDeviceWaitIdle (1) failed: '%s'", VulkanResultString(result, true));
            return false;
        }
        DINFO("Recreating swapchain, booting.");
        return false;
    }

    //Check if the framebuffer has been resized. If so, a new swapchain must be created.
    if(context.frameBufferSizeGeneration != context.frameBufferSizeLastGeneration){
        VkResult result = vkDeviceWaitIdle(device->logicalDevice);
        if(!VulkanResultIsSuccess(result)){
            DERROR("VulkanRendererBackendBeginFrame vkDeviceWaitIdle (2) failed: '%s'", VulkanResultString(result, true));
            return false;
        }
        //If swapchain recreation failed for something else like the window was minimised, boot out before unsetting the flag
        if(!RecreateSwapchain(backend)){
            return false;
        }

        DINFO("Resized, booting.");
        return false;
    }

    //Wait for the execution of the current fame to complete. The fence being free will allow this one to move on
    if(!VulkanFenceWait(&context, &context.inFlightFences[context.currentFrame], UINT64_MAX)){
        DWARN("In-flight fence wait failure!");
    }

    //Acquire the next image from the swap chain. Pass along the semaphore that should be signaled when this completes.
    //This same semaphore will later be waited on by the queue submission to ensure this image is available.
    if(!VulkanSwapchainAcquireNextImageIndex(
        &context, &context.swapchain, 
        UINT64_MAX, context.imageAvailableSemaphores[context.currentFrame], 
        0, &context.imageIndex)){
        return false;
    }

    //Begin recording commands
    VulkanCommandBuffer* commandBuffer = &context.graphicsCommandBuffers[context.imageIndex];
    VulkanCommandBufferReset(commandBuffer);
    VulkanCommandBufferBegin(commandBuffer, false, false, false);

    //Dynamic state
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = (f32)context.frameBufferHeight;
    viewport.width = (f32)context.frameBufferWidth;
    viewport.height = -(f32)context.frameBufferHeight; //render bottom up
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    //Scissor
    VkRect2D scissor = {};
    scissor.extent.width = context.frameBufferWidth;
    scissor.extent.height = context.frameBufferHeight;

    vkCmdSetViewport(commandBuffer->handle, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer->handle, 0, 1 , &scissor);

    context.mainRenderPass.w = context.frameBufferWidth;
    context.mainRenderPass.h = context. frameBufferHeight;

    //Begin the render pass
    VulkanRenderPassBegin(commandBuffer, &context.mainRenderPass, context.swapchain.frameBuffers[context.imageIndex].handle);

    return true;
}

b8 VulkanRendererBackendEndFrame(RendererBackend* backend, f32 deltaTime){
    VulkanCommandBuffer* commandBuffer = &context.graphicsCommandBuffers[context.imageIndex];

    //End renderpass
    VulkanRenderPassEnd(commandBuffer, &context.mainRenderPass);
    VulkanCommandBufferEnd(commandBuffer);

    //Make sure the previous frame is not using this image (the fence is being waited on)
    if(context.imagesInFlight[context.imageIndex] != VK_NULL_HANDLE){
        VulkanFenceWait(&context, context.imagesInFlight[context.imageIndex], UINT64_MAX);
    }

    //Mark image fance as in-use by this frame
    context.imagesInFlight[context.imageIndex] = &context.inFlightFences[context.currentFrame];

    //Reset the fence for sue on the next frame
    VulkanFenceReset(&context, &context.inFlightFences[context.currentFrame]);

    //Submit the queue and wait for the operatio to complete
    //Begin queue submission
    VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    //Command buffers to be executed
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer->handle;

    //The semaphores to be signaled when the queue is complete
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &context.queueCompleteSemaphores[context.currentFrame];

    //Wait semaphore ensures that the operation cannot begin until the image is available
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &context.imageAvailableSemaphores[context.currentFrame];

    //Each semaphore waits on the corresponding pipeline stage to complete. 1:1 ratio.
    //VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT prevents subsequent color attachment
    //writes from executing until the semaphore signals (one frame is presented at a time)
    VkPipelineStageFlags flags[1] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.pWaitDstStageMask = flags;

    VkResult result = vkQueueSubmit(context.device.graphicsQueue, 1, &submitInfo, context.inFlightFences[context.currentFrame].handle);
    if(result != VK_SUCCESS){
        DERROR("vkQueueSubmit failed with result: %s", VulkanResultString(result, true));
        return false;
    }

    VulkanCommandBufferUpdateSubmitted(commandBuffer);
    //End queue submission

    //Give the image back tot eh swapchain
    VulkanSwapchainPresent(
        &context,
        &context.swapchain,
        context.device.graphicsQueue,
        context.device.presentQueue,
        context.queueCompleteSemaphores[context.currentFrame],
        context.imageIndex);

    return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL vkDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
    void*  userData) {
        char* message = (char*)(callbackData->pMessage);
        switch(messageSeverity){
            default:
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                DERROR(message);
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                DWARN(message);
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                DINFO(message);
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                DTRACE(message);
                break;
        }
    return VK_FALSE;
}

i32 FindMemoryIndex(u32 typeFilter, u32 propertyFlags){
    VkPhysicalDeviceMemoryProperties memoryProperties = {};
    vkGetPhysicalDeviceMemoryProperties(context.device.physicalDevice, &memoryProperties);
    for(u32 i = 0; i < memoryProperties.memoryTypeCount; i++){
        //Check each memory type to see fi its bit is set to 1
        if(typeFilter & (1 << i) && (memoryProperties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags){
            return i;
        }
    }
    DWARN("Unable to find suitable memory type!");
    return -1;
}

void CreateCommandBuffers(RendererBackend* backend){
    //Need a separate command buffer for each swapchain image
    if(!context.graphicsCommandBuffers){
        context.graphicsCommandBuffers = 
            (VulkanCommandBuffer*)DarrayReserve(VulkanCommandBuffer, context.swapchain.imageCount);
        for(u32 i = 0; i < context.swapchain.imageCount; i++){
            DZeroMemory(&context.graphicsCommandBuffers[i], sizeof(VulkanCommandBuffer));
        }
    }

    for(u32 i = 0; i < context.swapchain.imageCount; i++){
        if(context.graphicsCommandBuffers[i].handle){
            VulkanCommandBufferFree(
                &context,
                context.device.graphicsCommandPool,
                &context.graphicsCommandBuffers[i]);
        }
        DZeroMemory(&context.graphicsCommandBuffers[i], sizeof(VulkanCommandBuffer));
        VulkanCommandBufferAllocate(
            &context,
            context.device.graphicsCommandPool,
            true,//always primary cmd buffer
            &context.graphicsCommandBuffers[i]);
    }
    DINFO("Vulkan command buffers created.");
}

void RegenerateFrameBuffers(RendererBackend* backend, VulkanSwapchain* swapchain, VulkanRenderPass* renderPass){
    //need framebuffer per swapchain image
    for(u32 i = 0; i < swapchain->imageCount; i++){
        //TODO: make this dynamic based on currently configured attachments
        u32 attachmentCount = 2;
        VkImageView attachments[] = {swapchain->views[i], swapchain->depthAttachment.view};

        VulkanFrameBufferCreate(
            &context,
            renderPass,
            context.frameBufferWidth,
            context.frameBufferHeight,
            attachmentCount,
            attachments,
            &context.swapchain.frameBuffers[i]);
    }
}

b8 RecreateSwapchain(RendererBackend* backend){
    //If already recreating, dont try again
    if(context.recreatingSwapchain){
        DDEBUG("RecreateSwapchain called when already recreating. Booting.");
        return false;
    }

    //Detect if the window is too small to be drawn to
    if(context.frameBufferWidth == 0 || context.frameBufferHeight == 0){
        DDEBUG("RecreateSwapchain called when window is < 1 in dimension. Booting.");
        return false;
    }

    //Mark as recreating if the dimension are valid
    context.recreatingSwapchain = true;
    //Wait for any operations to complete
    vkDeviceWaitIdle(context.device.logicalDevice);

    //clear the images out just in case
    for(u32 i = 0; i < context.swapchain.imageCount; i++){
        context.imagesInFlight[i] = 0;
    }

    //Requery support
    VulkanDeviceQuerySwapchainSupport(context.device.physicalDevice, context.surface, &context.device.swapchainSupport);
    VulkanDeviceDetectDepthFormat(&context.device);

    VulkanSwapchainRecreate(&context, cachedFrameBufferWidth, cachedFrameBufferHeight, &context.swapchain);

    //sync framebuffer size with the cached sizes
    context.frameBufferWidth= cachedFrameBufferWidth;
    context.frameBufferHeight = cachedFrameBufferHeight;
    context.mainRenderPass.w = context.frameBufferWidth;
    context.mainRenderPass.h = context.frameBufferHeight;
    cachedFrameBufferWidth = 0;
    cachedFrameBufferHeight = 0;

    //Update framebuffer size generation
    context.frameBufferSizeLastGeneration = context.frameBufferSizeGeneration;

    //Cleanup swapchain
    for(u32 i = 0; i < context.swapchain.imageCount; i++){
        VulkanCommandBufferFree(&context, context.device.graphicsCommandPool, &context.graphicsCommandBuffers[i]);
    }

    //Framebuffers
    for(u32 i = 0; i < context.swapchain.imageCount; i++){
        VulkanFrameBufferDestroy(&context, &context.swapchain.frameBuffers[i]);
    }

    context.mainRenderPass.x = 0;
    context.mainRenderPass.y = 0;
    context.mainRenderPass.w = context.frameBufferWidth;
    context.mainRenderPass.h = context.frameBufferHeight;

    RegenerateFrameBuffers(backend, &context.swapchain, &context.mainRenderPass);
    CreateCommandBuffers(backend);
    //clear the recreating flag
    context.recreatingSwapchain = false;
    return true;
}