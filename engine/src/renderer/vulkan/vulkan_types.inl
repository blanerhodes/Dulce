#pragma once

#include "defines.h"
#include "core/asserts.h"

#include "vulkan/vulkan.h"
#include "renderer/renderer_types.inl"

#define VK_CHECK(expr)             \
    {                              \
        DASSERT(expr == VK_SUCCESS)\
    }

struct VulkanBuffer {
    u64 total_size;
    VkBuffer handle;
    VkBufferUsageFlags usage;
    b8 is_locked;
    VkDeviceMemory memory;
    i32 memory_index;
    u32 memory_property_flags;
};

struct VulkanSwapchainSupportInfo{
    VkSurfaceCapabilitiesKHR capabilities;
    u32 format_count;
    VkSurfaceFormatKHR* formats;    
    u32 present_mode_count;
    VkPresentModeKHR* present_modes;
};

struct VulkanDevice{
    VkPhysicalDevice physical_device;
    VkDevice logical_device;
    VulkanSwapchainSupportInfo swapchain_support;
    i32 graphics_queue_index;
    i32 present_queue_index;
    i32 transfer_queue_index;
    VkQueue graphics_queue;
    VkQueue present_queue;
    VkQueue transfer_queue;
    VkCommandPool graphics_command_pool;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory;
    VkFormat depth_format;
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
    u32 attachment_count;
    VkImageView* attachments;
    VulkanRenderPass* render_pass;
};

struct VulkanSwapchain{
    VkSurfaceFormatKHR image_format;
    u8 max_frames_in_flight;
    VkSwapchainKHR handle;
    u32 image_count;
    VkImage* images;
    VkImageView* views;
    VulkanImage depth_attachment;
    //framebuffers used for on screen rendering
    VulkanFrameBuffer* frame_buffers;
};

enum VulkanCommandBufferState {
    COMMAND_BUFFER_STATE_NOT_ALLOCATED,
    COMMAND_BUFFER_STATE_READY,
    COMMAND_BUFFER_STATE_RECORDING,
    COMMAND_BUFFER_STATE_RECORDING_ENDED,
    COMMAND_BUFFER_STATE_SUBMITTED,
    COMMAND_BUFFER_STATE_IN_RENDER_PASS
};

struct VulkanCommandBuffer {
    VkCommandBuffer handle;
    VulkanCommandBufferState state;
};

struct VulkanFence {
    VkFence handle;
    b8 is_signaled;
};

struct VulkanShaderStage {
    VkShaderModuleCreateInfo create_info;
    VkShaderModule handle;
    VkPipelineShaderStageCreateInfo shader_stage_create_info;
};

struct VulkanPipeline {
    VkPipeline handle;
    VkPipelineLayout pipeline_layout;
};

//vert and frag
#define OBJECT_SHADER_STAGE_COUNT 2
struct VulkanObjectShader {
    VulkanShaderStage stages[OBJECT_SHADER_STAGE_COUNT];
    VkDescriptorPool global_descriptor_pool;
    VkDescriptorSetLayout global_descriptor_set_layout;
    //max 3 for triple buffering, each frame gets its own set
    VkDescriptorSet global_descriptor_sets[3];
    b8 descriptor_updated[3];
    GlobalUniformObject global_ubo;
    VulkanBuffer global_uniform_buffer;
    VulkanPipeline pipeline;
};

struct VulkanContext {
    u32 frame_buffer_width;
    u32 frame_buffer_height;
    //current generation of framebuffer size. If it doesn't matched framebufferSizeLastGeneration
    //a new one should be generate
    u64 frame_buffer_size_generation;
    //Generation of the framebuffer when it was last created. Set to framebufferSizeGeneration when updated
    u64 frame_buffer_size_last_generation;
    VkInstance instance;
    VkAllocationCallbacks* allocator;
    VkSurfaceKHR surface;
    VulkanDevice device;
    VulkanSwapchain swapchain;
    VulkanRenderPass main_render_pass;
    VulkanBuffer object_vertex_buffer;
    VulkanBuffer object_index_buffer;
    u64 geometry_vertex_offset;
    u64 geometry_index_offset;
    //darray
    VulkanCommandBuffer* graphics_command_buffers;
    //darray
    VkSemaphore* image_available_semaphores;
    //darray
    VkSemaphore* queue_complete_semaphores;
    u32 in_flight_fence_count;
    VulkanFence* in_flight_fences;
    //holds pointers to fences which exist and are ownde elsewhere
    VulkanFence** images_in_flight;

    u32 image_index;
    u32 current_frame;
    b8 recreating_swapchain;

    VulkanObjectShader object_shader;

#if defined(_DEBUG)
    VkDebugUtilsMessengerEXT debug_messenger;
#endif
    i32 (*FindMemoryIndex)(u32 type_filter, u32 property_flags);
};