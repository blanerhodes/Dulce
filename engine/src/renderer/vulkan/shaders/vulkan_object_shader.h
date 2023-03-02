#pragma once

#include "renderer/vulkan/vulkan_types.inl"
#include "renderer/renderer_types.inl"

b8 VulkanObjectShaderCreate(VulkanContext* context, VulkanObjectShader* out_shader);

void VulkanObjectShaderDestroy(VulkanContext* context, VulkanObjectShader* shader);

void VulkanObjectShaderUse(VulkanContext* context, VulkanObjectShader* shader);

void VulkanObjectShaderUpdateGlobalState(VulkanContext* context, VulkanObjectShader* shader);

void VulkanObjectShaderUpdateObject(VulkanContext* context, VulkanObjectShader* shader, Mat4 model);