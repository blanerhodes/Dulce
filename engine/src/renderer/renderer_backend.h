#pragma once

#include "renderer_types.inl"

struct PlatformState;

b8 RendererBackendCreate(RendererBackendType type, PlatformState* platState, RendererBackend* outRendererBackend);
void RendererBackendDestroy(RendererBackend* rendererBackend);