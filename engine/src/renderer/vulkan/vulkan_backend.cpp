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
#include "vulkan_buffer.h"
#include "vulkan_image.h"

#include "core/logger.h"
#include "core/dstring.h"
#include "core/dmemory.h"
#include "core/application.h"

#include "containers/darray.h"
#include "math/math_types.h"
#include "platform/platform.h"

#include "shaders/vulkan_object_shader.h"

static VulkanContext context;
static u32 cachedFrameBufferWidth;
static u32 cachedFrameBufferHeight;

VKAPI_ATTR VkBool32 VKAPI_CALL vkDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
    void*  userData);

i32 FindMemoryIndex(u32 typeFilter, u32 propertyFlags);
b8 CreateBuffers(VulkanContext* context);

void CreateCommandBuffers(RendererBackend* backend);
void RegenerateFrameBuffers(RendererBackend* backend, VulkanSwapchain* swapchain, VulkanRenderPass* renderPass);
b8 RecreateSwapchain(RendererBackend* backend);

void UploadDataRange(VulkanContext* context, VkCommandPool pool, VkFence fence, VkQueue queue, VulkanBuffer* buffer, u64 offset, u64 size, void* data) {
    //Create a host-visible staging buffer to upload to. Mark it as the source fo the transfer.
    VkBufferUsageFlags flags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VulkanBuffer staging = {};
    VulkanBufferCreate(context, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, flags, true, &staging);

    //Load the data into the staging buffer
    VulkanBufferLoadData(context, &staging, 0, size, 0, data);

    //Copy from staging to device local buffer
    VulkanBufferCopyTo(context, pool, fence, queue, staging.handle, 0, buffer->handle, offset, size);

    //Clean up staging buffer
    VulkanBufferDestroy(context, &staging);
}

b8 VulkanRendererBackendInitialize(RendererBackend* backend, char* applicationName){
    //Function pointers
    context.FindMemoryIndex = FindMemoryIndex;

    //TODO: custom allocator
    context.allocator = 0;

    ApplicationGetFrameBufferSize(&cachedFrameBufferWidth, &cachedFrameBufferHeight);
    context.frame_buffer_width = (cachedFrameBufferWidth != 0) ? cachedFrameBufferWidth : 1280;
    context.frame_buffer_height = (cachedFrameBufferHeight != 0) ? cachedFrameBufferHeight : 720;
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
    VK_CHECK(createVkDebugger(context.instance, &debugCreateInfo, context.allocator, &context.debug_messenger));
    DDEBUG("Vulkan debugger created.");
#endif

    //Surface
    DDEBUG("Creating Vulkan surface...");
    if(!PlatformCreateVulkanSurface(&context)){
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
    VulkanSwapchainCreate(&context, context.frame_buffer_width, context.frame_buffer_height, &context.swapchain);
    DINFO("Vulkan swapchain created.");

    //Renderpass
    VulkanRenderPassCreate(
        &context,
        &context.main_render_pass,
        0, 0, context.frame_buffer_width, context.frame_buffer_height,
        0.0f, 0.0f, 0.2f, 1.0f,
        1.0f, 0);

    //Swapchain framebuffers
    context.swapchain.frame_buffers = (VulkanFrameBuffer*)DarrayReserve(VulkanFrameBuffer, context.swapchain.image_count);
    RegenerateFrameBuffers(backend, &context.swapchain, &context.main_render_pass);

    //Create command buffers
    CreateCommandBuffers(backend);

    //Create sync objects
    context.image_available_semaphores = (VkSemaphore*)DarrayReserve(VkSemaphore, context.swapchain.max_frames_in_flight);
    context.queue_complete_semaphores = (VkSemaphore*)DarrayReserve(VkSemaphore, context.swapchain.max_frames_in_flight);
    context.in_flight_fences = (VulkanFence*)DarrayReserve(VulkanFence, context.swapchain.max_frames_in_flight);

    for(u8 i = 0; i < context.swapchain.max_frames_in_flight; i++){
        VkSemaphoreCreateInfo semaphoreCreateInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        vkCreateSemaphore(context.device.logical_device, &semaphoreCreateInfo,
            context.allocator, &context.image_available_semaphores[i]);
        vkCreateSemaphore(context.device.logical_device, &semaphoreCreateInfo,
            context.allocator, &context.queue_complete_semaphores[i]);

        //Create the fence in a signaled state, indicating that the first frame ahs already been "rendered".
        //This will prevent the application form waiting indefinitely for the first frame to render since it
        //cannot be rendered until a frame is "rendered" before it.
        VulkanFenceCreate(&context, true, &context.in_flight_fences[i]);
    }

    //In flight fences should nto yet exist at this point, so clera the list. These are stored in pointers
    //because the intial state should be 0, and will be 0 when not in use. Actual fences are not owned
    //by this list.
    context.images_in_flight = (VulkanFence**)DarrayReserve(VulkanFence, context.swapchain.image_count);
    for(u32 i = 0; i < context.swapchain.image_count; i++){
        context.images_in_flight[i] = 0;
    }

    //Create builtin shaders
    if (!VulkanObjectShaderCreate(&context, &context.object_shader)) {
        DERROR("Error loading built-in basic_lighting shader");
        return false;
    }

    CreateBuffers(&context);

    //TODO: tempt test code
    const u32 vert_count = 4;
    Vertex3d verts[vert_count] = {};

    const f32 f = 10.0f;

    verts[0].position.x = -0.5 * f;
    verts[0].position.y = -0.5 * f;
    verts[0].texcoord.x = 0.0;
    verts[0].texcoord.y = 0.0;

    verts[1].position.x = 0.5 * f;
    verts[1].position.y = 0.5 * f;
    verts[1].texcoord.x = 1.0;
    verts[1].texcoord.y = 1.0;

    verts[2].position.x = -0.5 * f;
    verts[2].position.y = 0.5 * f;
    verts[2].texcoord.x = 0.0;
    verts[2].texcoord.y = 1.0;

    verts[3].position.x = 0.5 * f;
    verts[3].position.y = -0.5 * f;
    verts[3].texcoord.x = 1.0;
    verts[3].texcoord.y = 0.0;

    const u32 index_count = 6;
    u32 indices[index_count] = {0, 1, 2, 0, 3, 1};

    UploadDataRange(&context, context.device.graphics_command_pool, 0, context.device.graphics_queue,
                    &context.object_vertex_buffer, 0, sizeof(verts), verts);
    UploadDataRange(&context, context.device.graphics_command_pool, 0, context.device.graphics_queue,
                    &context.object_index_buffer, 0, sizeof(indices), indices);
    u32 object_id = 0;
    if (!VulkanObjectShaderAcquireResources(&context, &context.object_shader, &object_id)) {
        DERROR("Failed to acquire shader resrouces");
        return false;
    }
    //end temp code
    
    DINFO("Vulkan renderer initialized successfully.");
    return true;
}

void VulkanRendererBackendShutdown(RendererBackend* backend){
    vkDeviceWaitIdle(context.device.logical_device);
    //destroy in opposite order of creation

    VulkanBufferDestroy(&context, &context.object_vertex_buffer);
    VulkanBufferDestroy(&context, &context.object_index_buffer);

    VulkanObjectShaderDestroy(&context, &context.object_shader);

    //sync objects
    for(u8 i = 0; i < context.swapchain.max_frames_in_flight; i++){
        if(context.image_available_semaphores[i]){
            vkDestroySemaphore(
                context.device.logical_device,
                context.image_available_semaphores[i],
                context.allocator);
            context.image_available_semaphores[i] = 0;
        }
        if(context.queue_complete_semaphores[i]){
            vkDestroySemaphore(
                context.device.logical_device,
                context.queue_complete_semaphores[i],
                context.allocator);
            context.queue_complete_semaphores[i] = 0;
        }
        VulkanFenceDestroy(&context, &context.in_flight_fences[i]);
    }
    DarrayDestroy(context.image_available_semaphores);
    context.image_available_semaphores = 0;
    DarrayDestroy(context.queue_complete_semaphores);
    context.queue_complete_semaphores = 0;
    DarrayDestroy(context.in_flight_fences);
    context.in_flight_fences = 0;

    //Command buffers
    for(u32 i = 0; i < context.swapchain.image_count; i++){
        if(context.graphics_command_buffers[i].handle){
            VulkanCommandBufferFree(&context, context.device.graphics_command_pool, &context.graphics_command_buffers[i]);
            context.graphics_command_buffers[i].handle = 0;
        }
    }
    DarrayDestroy(context.graphics_command_buffers);
    context.graphics_command_buffers = 0;

    //Destroy framebuffers
    for(u32 i = 0; i < context.swapchain.image_count; i++){
        VulkanFrameBufferDestroy(&context, &context.swapchain.frame_buffers[i]);
    }

    //Renderpass
    VulkanRenderPassDestroy(&context, &context.main_render_pass);

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
    if(context.debug_messenger){
        PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, "vkDestroyDebugUtilsMessengerEXT");
        func(context.instance, context.debug_messenger, context.allocator);
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
    context.frame_buffer_size_generation++;
    DINFO("Vulkan renderer backend->resized: w/h/gen: %i/%i/%llu", width, height, context.frame_buffer_size_generation);
}

b8 VulkanRendererBackendBeginFrame(RendererBackend* backend, f32 delta_time) {
    context.frame_delta_time = delta_time;
    VulkanDevice* device = &context.device;

    //Check if recrating swap chain and boot out
    if(context.recreating_swapchain){
        VkResult result = vkDeviceWaitIdle(device->logical_device);
        if(!VulkanResultIsSuccess(result)){
            DERROR("VulkanRendererBackendBeginFrame vkDeviceWaitIdle (1) failed: '%s'", VulkanResultString(result, true));
            return false;
        }
        DINFO("Recreating swapchain, booting.");
        return false;
    }

    //Check if the framebuffer has been resized. If so, a new swapchain must be created.
    if(context.frame_buffer_size_generation != context.frame_buffer_size_last_generation){
        VkResult result = vkDeviceWaitIdle(device->logical_device);
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
    if(!VulkanFenceWait(&context, &context.in_flight_fences[context.current_frame], UINT64_MAX)){
        DWARN("In-flight fence wait failure!");
    }

    //Acquire the next image from the swap chain. Pass along the semaphore that should be signaled when this completes.
    //This same semaphore will later be waited on by the queue submission to ensure this image is available.
    if(!VulkanSwapchainAcquireNextImageIndex(
        &context, &context.swapchain, 
        UINT64_MAX, context.image_available_semaphores[context.current_frame], 
        0, &context.image_index)){
        return false;
    }

    //Begin recording commands
    VulkanCommandBuffer* command_buffer = &context.graphics_command_buffers[context.image_index];
    VulkanCommandBufferReset(command_buffer);
    VulkanCommandBufferBegin(command_buffer, false, false, false);

    //Dynamic state
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = (f32)context.frame_buffer_height;
    viewport.width = (f32)context.frame_buffer_width;
    viewport.height = -(f32)context.frame_buffer_height; //render bottom up
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    //Scissor
    VkRect2D scissor = {};
    scissor.extent.width = context.frame_buffer_width;
    scissor.extent.height = context.frame_buffer_height;

    vkCmdSetViewport(command_buffer->handle, 0, 1, &viewport);
    vkCmdSetScissor(command_buffer->handle, 0, 1 , &scissor);

    context.main_render_pass.w = context.frame_buffer_width;
    context.main_render_pass.h = context. frame_buffer_height;

    //Begin the render pass
    VulkanRenderPassBegin(command_buffer, &context.main_render_pass, context.swapchain.frame_buffers[context.image_index].handle);


    return true;
}

void VulkanRendererBackendUpdateGlobalState(Mat4 projection, Mat4 view, Vec3 view_position, Vec4 ambient_color, i32 mode) {

    VulkanObjectShaderUse(&context, &context.object_shader);

    context.object_shader.global_ubo.projection = projection;
    context.object_shader.global_ubo.view = view;
    //TODO: other ubo props

    VulkanObjectShaderUpdateGlobalState(&context, &context.object_shader, context.frame_delta_time);

}

b8 VulkanRendererBackendEndFrame(RendererBackend* backend, f32 delta_time){
    VulkanCommandBuffer* commandBuffer = &context.graphics_command_buffers[context.image_index];

    //End renderpass
    VulkanRenderPassEnd(commandBuffer, &context.main_render_pass);
    VulkanCommandBufferEnd(commandBuffer);

    //Make sure the previous frame is not using this image (the fence is being waited on)
    if(context.images_in_flight[context.image_index] != VK_NULL_HANDLE){
        VulkanFenceWait(&context, context.images_in_flight[context.image_index], UINT64_MAX);
    }

    //Mark image fance as in-use by this frame
    context.images_in_flight[context.image_index] = &context.in_flight_fences[context.current_frame];

    //Reset the fence for sue on the next frame
    VulkanFenceReset(&context, &context.in_flight_fences[context.current_frame]);

    //Submit the queue and wait for the operatio to complete
    //Begin queue submission
    VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    //Command buffers to be executed
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer->handle;

    //The semaphores to be signaled when the queue is complete
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &context.queue_complete_semaphores[context.current_frame];

    //Wait semaphore ensures that the operation cannot begin until the image is available
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &context.image_available_semaphores[context.current_frame];

    //Each semaphore waits on the corresponding pipeline stage to complete. 1:1 ratio.
    //VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT prevents subsequent color attachment
    //writes from executing until the semaphore signals (one frame is presented at a time)
    VkPipelineStageFlags flags[1] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.pWaitDstStageMask = flags;

    VkResult result = vkQueueSubmit(context.device.graphics_queue, 1, &submitInfo, context.in_flight_fences[context.current_frame].handle);
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
        context.device.graphics_queue,
        context.device.present_queue,
        context.queue_complete_semaphores[context.current_frame],
        context.image_index);

    return true;
}

void VulkanRendererBackendUpdateObject(GeometryRenderData data) {
    VulkanObjectShaderUpdateObject(&context, &context.object_shader, data);

    VulkanCommandBuffer* command_buffer = &context.graphics_command_buffers[context.image_index];
    //TODO: temp test code
    VulkanObjectShaderUse(&context, &context.object_shader);

    VkDeviceSize offsets[1] = {0};
    vkCmdBindVertexBuffers(command_buffer->handle, 0, 1, &context.object_vertex_buffer.handle, (VkDeviceSize*)offsets);
    vkCmdBindIndexBuffer(command_buffer->handle, context.object_index_buffer.handle, 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(command_buffer->handle, 6, 1, 0, 0, 0);
    //end temp test code
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
    vkGetPhysicalDeviceMemoryProperties(context.device.physical_device, &memoryProperties);
    for(u32 i = 0; i < memoryProperties.memoryTypeCount; i++){
        //Check each memory type to see if its bit is set to 1
        if(typeFilter & (1 << i) && (memoryProperties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags){
            return i;
        }
    }
    DWARN("Unable to find suitable memory type!");
    return -1;
}

void CreateCommandBuffers(RendererBackend* backend){
    //Need a separate command buffer for each swapchain image
    if(!context.graphics_command_buffers){
        context.graphics_command_buffers = 
            (VulkanCommandBuffer*)DarrayReserve(VulkanCommandBuffer, context.swapchain.image_count);
        for(u32 i = 0; i < context.swapchain.image_count; i++){
            DZeroMemory(&context.graphics_command_buffers[i], sizeof(VulkanCommandBuffer));
        }
    }

    for(u32 i = 0; i < context.swapchain.image_count; i++){
        if(context.graphics_command_buffers[i].handle){
            VulkanCommandBufferFree(
                &context,
                context.device.graphics_command_pool,
                &context.graphics_command_buffers[i]);
        }
        DZeroMemory(&context.graphics_command_buffers[i], sizeof(VulkanCommandBuffer));
        VulkanCommandBufferAllocate(
            &context,
            context.device.graphics_command_pool,
            true,//always primary cmd buffer
            &context.graphics_command_buffers[i]);
    }
    DINFO("Vulkan command buffers created.");
}

void RegenerateFrameBuffers(RendererBackend* backend, VulkanSwapchain* swapchain, VulkanRenderPass* renderPass){
    //need framebuffer per swapchain image
    for(u32 i = 0; i < swapchain->image_count; i++){
        //TODO: make this dynamic based on currently configured attachments
        u32 attachmentCount = 2;
        VkImageView attachments[] = {swapchain->views[i], swapchain->depth_attachment.view};

        VulkanFrameBufferCreate(
            &context,
            renderPass,
            context.frame_buffer_width,
            context.frame_buffer_height,
            attachmentCount,
            attachments,
            &context.swapchain.frame_buffers[i]);
    }
}

b8 RecreateSwapchain(RendererBackend* backend){
    //If already recreating, dont try again
    if(context.recreating_swapchain){
        DDEBUG("RecreateSwapchain called when already recreating. Booting.");
        return false;
    }

    //Detect if the window is too small to be drawn to
    if(context.frame_buffer_width == 0 || context.frame_buffer_height == 0){
        DDEBUG("RecreateSwapchain called when window is < 1 in dimension. Booting.");
        return false;
    }

    //Mark as recreating if the dimension are valid
    context.recreating_swapchain = true;
    //Wait for any operations to complete
    vkDeviceWaitIdle(context.device.logical_device);

    //clear the images out just in case
    for(u32 i = 0; i < context.swapchain.image_count; i++){
        context.images_in_flight[i] = 0;
    }

    //Requery support
    VulkanDeviceQuerySwapchainSupport(context.device.physical_device, context.surface, &context.device.swapchain_support);
    VulkanDeviceDetectDepthFormat(&context.device);

    VulkanSwapchainRecreate(&context, cachedFrameBufferWidth, cachedFrameBufferHeight, &context.swapchain);

    //sync framebuffer size with the cached sizes
    context.frame_buffer_width= cachedFrameBufferWidth;
    context.frame_buffer_height = cachedFrameBufferHeight;
    context.main_render_pass.w = context.frame_buffer_width;
    context.main_render_pass.h = context.frame_buffer_height;
    cachedFrameBufferWidth = 0;
    cachedFrameBufferHeight = 0;

    //Update framebuffer size generation
    context.frame_buffer_size_last_generation = context.frame_buffer_size_generation;

    //Cleanup swapchain
    for(u32 i = 0; i < context.swapchain.image_count; i++){
        VulkanCommandBufferFree(&context, context.device.graphics_command_pool, &context.graphics_command_buffers[i]);
    }

    //Framebuffers
    for(u32 i = 0; i < context.swapchain.image_count; i++){
        VulkanFrameBufferDestroy(&context, &context.swapchain.frame_buffers[i]);
    }

    context.main_render_pass.x = 0;
    context.main_render_pass.y = 0;
    context.main_render_pass.w = context.frame_buffer_width;
    context.main_render_pass.h = context.frame_buffer_height;

    RegenerateFrameBuffers(backend, &context.swapchain, &context.main_render_pass);
    CreateCommandBuffers(backend);
    //clear the recreating flag
    context.recreating_swapchain = false;
    return true;
}

b8 CreateBuffers(VulkanContext* context) {
    VkMemoryPropertyFlagBits memory_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    u64 vertex_buffer_size = sizeof(Vertex3d) * 1024 * 1024;
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT; 
    if (!VulkanBufferCreate(context, vertex_buffer_size, usage,
                            memory_property_flags, true, &context->object_vertex_buffer)) {
        DERROR("Error creating vertex buffer.");
        return false;
    }
    context->geometry_vertex_offset = 0;

    u64 index_buffer_size = sizeof(u32) * 1024 * 1024;
    VkBufferUsageFlags index_usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT; 
    if (!VulkanBufferCreate(context, index_buffer_size, index_usage,
                            memory_property_flags, true, &context->object_index_buffer)) {
        DERROR("Error creating index buffer.");
        return false;
    }
    context->geometry_index_offset = 0;

    return true;
}

void VulkanRendererCreateTexture(char* name, b8 auto_release, i32 width, i32 height, i32 channel_count,
                                 u8* pixels, b8 has_transparency, Texture* out_texture) {
    out_texture->width = width;
    out_texture->height = height;
    out_texture->channel_count = channel_count;
    out_texture->generation = INVALID_ID;

    //internal data creation
    //TODO: use an allocator for this
    out_texture->internal_data = (VulkanTextureData*)DAllocate(sizeof(VulkanTextureData), MEMORY_TAG_TEXTURE);
    VulkanTextureData* data = (VulkanTextureData*)out_texture->internal_data;
    VkDeviceSize image_size = width * height * channel_count;

    //NOTE: assumes 8 bits per channel
    VkFormat image_format = VK_FORMAT_R8G8B8A8_UNORM;

    //Create a staging buffer and load data into it
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VkMemoryPropertyFlags memory_prop_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VulkanBuffer staging = {};
    VulkanBufferCreate(&context, image_size, usage, memory_prop_flags, true, &staging);

    VulkanBufferLoadData(&context, &staging, 0, image_size, 0, pixels);

    VulkanImageCreate(&context, VK_IMAGE_TYPE_2D, width, height, image_format, VK_IMAGE_TILING_OPTIMAL,
                      VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true, VK_IMAGE_ASPECT_COLOR_BIT, &data->image);

    VulkanCommandBuffer temp_cmd_buffer = {};
    VkCommandPool pool = context.device.graphics_command_pool;
    VkQueue queue = context.device.graphics_queue;
    VulkanCommandBufferAllocateAndBeginSingleUse(&context, pool, &temp_cmd_buffer);
    
    //transition from whatever the layout is currently to optimal for receiving data
    VulkanImageTransitionLayout(&context, &temp_cmd_buffer, &data->image, image_format, 
                                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    //copy the data from the buffer
    VulkanImageCopyFromBuffer(&context, &data->image, staging.handle, &temp_cmd_buffer);
    //transition from optimal for data receiving to shader-read-only so it's readable by the shader
    VulkanImageTransitionLayout(&context, &temp_cmd_buffer, &data->image, image_format, 
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            
    VulkanCommandBufferEndSingleUse(&context, pool, &temp_cmd_buffer, queue);
    VulkanBufferDestroy(&context, &staging);


    VkSamplerCreateInfo sampler_info = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    //TODO: filters should be configurable
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.anisotropyEnable = VK_TRUE;
    sampler_info.maxAnisotropy = 16;
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = 0.0f;

    VkResult result = vkCreateSampler(context.device.logical_device, &sampler_info, context.allocator, &data->sampler);
    if (!VulkanResultIsSuccess(result)) {
        DERROR("Error creating texture sampler: %s", VulkanResultString(result, true));
        return;
    }

    out_texture->has_transparency = has_transparency;
    out_texture->generation++;
}

void VulkanRendererDestroyTexture(Texture* texture) {
    vkDeviceWaitIdle(context.device.logical_device);
    VulkanTextureData* data = (VulkanTextureData*)texture->internal_data;

    VulkanImageDestroy(&context, &data->image);
    DZeroMemory(&data->image, sizeof(VulkanImage));
    vkDestroySampler(context.device.logical_device, data->sampler, context.allocator);
    data->sampler = 0;

    DFree(texture->internal_data, sizeof(VulkanTextureData), MEMORY_TAG_TEXTURE);
    DZeroMemory(texture, sizeof(Texture));
}