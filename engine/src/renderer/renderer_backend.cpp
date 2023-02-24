#include "renderer_backend.h"
#include "vulkan/vulkan_backend.h"

b8 RendererBackendCreate(RendererBackendType type, PlatformState* platState, RendererBackend* outRendererBackend){
    outRendererBackend->platState = platState;

    if(type == RENDERER_BACKEND_TYPE_VULKAN){
        outRendererBackend->Initialize = VulkanRendererBackendInitialize;
        outRendererBackend->Shutdown = VulkanRendererBackendShutdown;
        outRendererBackend->BeginFrame = VulkanRendererBackendBeginFrame;
        outRendererBackend->EndFrame = VulkanRendererBackendEndFrame;
        outRendererBackend->Resized = VulkanRendererBackendOnResized;
        return true;
    }

    return false;
}

void RendererBackendDestroy(RendererBackend* rendererBackend){
    rendererBackend->Initialize = 0;
    rendererBackend->Shutdown = 0;
    rendererBackend->BeginFrame = 0;
    rendererBackend->EndFrame = 0;
    rendererBackend->Resized = 0;
}