#pragma once

#include "renderer/renderer_backend.h"


b8 VulkanRendererBackendInitialize(RendererBackend* backend, char* applicationName);

void VulkanRendererBackendShutdown(RendererBackend* backend);

void VulkanRendererBackendOnResized(RendererBackend* backend, u16 width, u16 height);

b8 VulkanRendererBackendBeginFrame(RendererBackend* backend, f32 delta_time);

void VulkanRendererBackendUpdateGlobalState(Mat4 projection, Mat4 view, Vec3 view_position, Vec4 ambient_color, i32 mode);

b8 VulkanRendererBackendEndFrame(RendererBackend* backend, f32 delta_time);

void VulkanRendererBackendUpdateObject(Mat4 model);