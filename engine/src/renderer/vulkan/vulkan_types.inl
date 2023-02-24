#pragma once

#include "defines.h"
#include "core/asserts.h"

#include "vulkan/vulkan.h"

#define VK_CHECK(expr)             \
    {                              \
        DASSERT(expr == VK_SUCCESS)\
    }

struct VulkanSwapchainSupportInfo{
    VkSurfaceCapabilitiesKHR capabilities;
    u32 formatCount;
    VkSurfaceFormatKHR* formats;    
    u32 presentModeCount;
    VkPresentModeKHR* presentModes;
};

struct VulkanDevice{
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
    VulkanSwapchainSupportInfo swapchainSupport;
    i32 graphicsQueueIndex;
    i32 presentQueueIndex;
    i32 transferQueueIndex;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue transferQueue;
    VkCommandPool graphicsCommandPool;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory;
    VkFormat depthFormat;
};

struct VulkanImage{
    VkImage handle;
    VkDeviceMemory memory;
    VkImageView view;
    u32 width;
    u32 height;
};

enum VulkanRenderPassState{
    READY,
    RECORDING,
    IN_RENDER_PASS,
    RECORDING_ENDED,
    SUBMITTED,
    NOT_ALLOCATED
};

struct VulkanRenderPass{
    VkRenderPass handle;
    f32 x, y, w, h;
    f32 r, g, b, a;
    f32 depth;
    f32 stencil;
    VulkanRenderPassState state;
};

struct VulkanFrameBuffer{
    VkFramebuffer handle;
    u32 attachmentCount;
    VkImageView* attachments;
    VulkanRenderPass* renderPass;
};

struct VulkanSwapchain{
    VkSurfaceFormatKHR imageFormat;
    u8 maxFramesInFlight;
    VkSwapchainKHR handle;
    u32 imageCount;
    VkImage* images;
    VkImageView* views;
    VulkanImage depthAttachment;
    //framebuffers used for on screen rendering
    VulkanFrameBuffer* frameBuffers;
};

enum VulkanCommandBufferState{
    COMMAND_BUFFER_STATE_NOT_ALLOCATED,
    COMMAND_BUFFER_STATE_READY,
    COMMAND_BUFFER_STATE_RECORDING,
    COMMAND_BUFFER_STATE_RECORDING_ENDED,
    COMMAND_BUFFER_STATE_SUBMITTED,
    COMMAND_BUFFER_STATE_IN_RENDER_PASS
};

struct VulkanCommandBuffer{
    VkCommandBuffer handle;
    VulkanCommandBufferState state;
};

struct VulkanFence{
    VkFence handle;
    b8 isSignaled;
};

struct VulkanContext {
    u32 frameBufferWidth;
    u32 frameBufferHeight;
    //current generation of framebuffer size. If it doesn't matched framebufferSizeLastGeneration
    //a new one should be generate
    u64 frameBufferSizeGeneration;
    //Generation of the framebuffer when it was last created. Set to framebufferSizeGeneration when updated
    u64 frameBufferSizeLastGeneration;
    VkInstance instance;
    VkAllocationCallbacks* allocator;
    VkSurfaceKHR surface;
    VulkanDevice device;
    VulkanSwapchain swapchain;
    VulkanRenderPass mainRenderPass;
    //darray
    VulkanCommandBuffer* graphicsCommandBuffers;
    //darray
    VkSemaphore* imageAvailableSemaphores;
    //darray
    VkSemaphore* queueCompleteSemaphores;
    u32 inFlightFenceCount;
    VulkanFence* inFlightFences;
    //holds pointers to fences which exist and are ownde elsewhere
    VulkanFence** imagesInFlight;

    u32 imageIndex;
    u32 currentFrame;
    b8 recreatingSwapchain;

#if defined(_DEBUG)
    VkDebugUtilsMessengerEXT debugMessenger;
#endif
    i32 (*FindMemoryIndex)(u32 typeFilter, u32 propertyFlags);
};