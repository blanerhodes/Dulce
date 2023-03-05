#pragma once

#include "renderer_types.inl"

struct PlatformState;

b8 RendererBackendCreate(RendererBackendType type,  RendererBackend* outRendererBackend);
void RendererBackendDestroy(RendererBackend* rendererBackend);

void VulkanRendererCreateTexture(char* name, b8 auto_release, i32 width, i32 height, i32 channel_count,
                                 u8* pixels, b8 has_transparency, Texture* out_texture);

void VulkanRendererDestroyTexture(Texture* texture);