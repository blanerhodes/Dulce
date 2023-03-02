//core
#include "core/clock.cpp"
#include "core/logger.cpp"
#include "containers/darray.cpp"
#include "core/dmemory.cpp"
#include "core/dstring.cpp"
#include "core/event.cpp"
#include "core/input.cpp"
#include "core/application.cpp"

//math
#include "math/dmath.cpp"

//memory
#include "memory/linear_allocator.cpp"

//platform
#include "platform/filesystem.cpp"
#include "platform/platform_win32.cpp"

//renderer
#include "renderer/vulkan/shaders/vulkan_object_shader.cpp"
#include "renderer/vulkan/vulkan_backend.cpp"
#include "renderer/vulkan/vulkan_buffer.cpp"
#include "renderer/vulkan/vulkan_command_buffer.cpp"
#include "renderer/vulkan/vulkan_device.cpp"
#include "renderer/vulkan/vulkan_fence.cpp"
#include "renderer/vulkan/vulkan_framebuffer.cpp"
#include "renderer/vulkan/vulkan_image.cpp"
#include "renderer/vulkan/vulkan_pipeline.cpp"
#include "renderer/vulkan/vulkan_renderpass.cpp"
#include "renderer/vulkan/vulkan_shader_utils.cpp"
#include "renderer/vulkan/vulkan_swapchain.cpp"
#include "renderer/vulkan/vulkan_utils.cpp"
#include "renderer/renderer_backend.cpp"
#include "renderer/renderer_frontend.cpp"