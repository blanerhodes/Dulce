#include "renderer_frontend.h"
#include "renderer_backend.h"
#include "core/logger.h"
#include "core/dmemory.h"

static RendererBackend* backend = 0;

b8 RendererIntialize(char* applicationName, PlatformState* platState){
    backend = (RendererBackend*)DAllocate(sizeof(RendererBackend), MEMORY_TAG_RENDERER);

    RendererBackendCreate(RENDERER_BACKEND_TYPE_VULKAN, platState, backend);
    backend->frameNumber = 0;

    if(!backend->Initialize(backend, applicationName, platState)){
        DFATAL("Renderer backend failed to initialize. Shutting down.");
        return false;
    }

    return true;
}

void RendererShutdown(){
    backend->Shutdown(backend);
    DFree(backend, sizeof(RendererBackend), MEMORY_TAG_RENDERER);
}

b8 RendererBeginFrame(f32 deltaTime){
    return backend->BeginFrame(backend, deltaTime);
}

b8 RendererEndFrame(f32 deltaTime){
    b8 result = backend->EndFrame(backend, deltaTime);
    backend->frameNumber++;
    return result;
}

void RendererOnResized(u16 width, u16 height){
    if(backend){
        backend->Resized(backend, width, height);
    } else{
        DWARN("Renderer Backend does not exist to accept resize: %i, %i", width, height);
    }
}

b8 RendererDrawFrame(RenderPacket* packet){
    if(RendererBeginFrame(packet->deltaTime)){
        b8 result = RendererEndFrame(packet->deltaTime);
        if(!result){
            DERROR("RendererEndFrame failed. Application shutting down.");
            return false;
        }
    }
    return true;
}