#pragma once

#include "renderer_types.inl"

b8 RendererSystemInitialize(u64* memory_requirement, void* state, char* application_name);
void RendererSystemShutdown(void* state);

void RendererOnResized(u16 width, u16 height);

b8 RendererDrawFrame(struct RenderPacket* packet);

//temp export
DAPI void RendererSetView(Mat4 view);


void RendererCreateTexture(char* name, b8 auto_release, i32 width, i32 height, i32 channel_count,
                      u8* pixels, b8 has_transparency, Texture* out_texture);

void RendererDestroyTexture(Texture* texture);