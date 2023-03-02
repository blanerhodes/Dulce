#pragma once

#include "renderer_types.inl"

b8 RendererSystemInitialize(u64* memory_requirement, void* state, char* application_name);
void RendererSystemShutdown(void* state);

void RendererOnResized(u16 width, u16 height);

b8 RendererDrawFrame(struct RenderPacket* packet);

//temp export
DAPI void RendererSetView(Mat4 view);