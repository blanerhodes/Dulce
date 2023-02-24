#pragma once

#include "defines.h"

enum RendererBackendType{
    RENDERER_BACKEND_TYPE_VULKAN,
    RENDERER_BACKEND_TYPE_OPENGL,
    RENDERER_BACKEND_TYPE_DIRECTX
};

struct RendererBackend{
    struct PlatformState* platState;

    b8 (*Initialize)(struct RendererBackend* backend, char* applicationName, struct PlatformState* platState);

    void (*Shutdown)(struct RendererBackend* backend);

    void (*Resized)(struct RendererBackend* backend, u16 width, u16 height);

    b8 (*BeginFrame)(struct RendererBackend* backend, f32 deltaTime);
    b8 (*EndFrame)(struct RendererBackend* backend, f32 deltaTime);

    u32 frameNumber;
};

struct RenderPacket{
    f32 deltaTime;
};