#pragma once

#include "renderer_types.inl"

struct StaticMeshData;
struct PlatformState;

b8 RendererIntialize(char* applicationName,  struct PlatformState* platState);
void RendererShutdown();

void RendererOnResized(u16 width, u16 height);

b8 RendererDrawFrame(struct RenderPacket* packet);