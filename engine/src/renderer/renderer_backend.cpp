#include "renderer_backend.h"
#include "vulkan/vulkan_backend.h"

b8 RendererBackendCreate(RendererBackendType type, RendererBackend* out_renderer_backend){
    if(type == RENDERER_BACKEND_TYPE_VULKAN){
        out_renderer_backend->Initialize = VulkanRendererBackendInitialize;
        out_renderer_backend->Shutdown = VulkanRendererBackendShutdown;
        out_renderer_backend->BeginFrame = VulkanRendererBackendBeginFrame;
        out_renderer_backend->UpdateGlobalState = VulkanRendererBackendUpdateGlobalState;
        out_renderer_backend->EndFrame = VulkanRendererBackendEndFrame;
        out_renderer_backend->Resized = VulkanRendererBackendOnResized;
        out_renderer_backend->UpdateObject = VulkanRendererBackendUpdateObject;
        return true;
    }
    return false;
}

void RendererBackendDestroy(RendererBackend* renderer_backend){
    renderer_backend->Initialize = 0;
    renderer_backend->Shutdown = 0;
    renderer_backend->BeginFrame = 0;
    renderer_backend->UpdateGlobalState = 0;
    renderer_backend->EndFrame = 0;
    renderer_backend->Resized = 0;
    renderer_backend->UpdateObject = 0;
}