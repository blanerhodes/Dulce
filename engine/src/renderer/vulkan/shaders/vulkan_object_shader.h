#pragma once

#include "renderer/vulkan/vulkan_types.inl"
#include "renderer/renderer_types.inl"

b8 VulkanObjectShaderCreate(VulkanContext* context, VulkanObjectShader* out_shader);

void VulkanObjectShaderDestroy(VulkanContext* context, VulkanObjectShader* shader);

void VulkanObjectShaderUse(VulkanContext* context, VulkanObjectShader* shader);

void VulkanObjectShaderUpdateGlobalState(VulkanContext* context, VulkanObjectShader* shader, f32 delta_time);

void VulkanObjectShaderUpdateObject(VulkanContext* context, VulkanObjectShader* shader, GeometryRenderData data);

b8 VulkanObjectShaderAcquireResources(VulkanContext* context, VulkanObjectShader* shader, u32* out_object_id);

void VulkanObjectShaderReleaseResources(VulkanContext* context, VulkanObjectShader* shader, u32 object_id);