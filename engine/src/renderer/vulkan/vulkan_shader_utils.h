#pragma once

#include "vulkan_types.inl"

b8 CreateShaderModule(VulkanContext* context, char* name, char* type_str, 
                      VkShaderStageFlagBits stage_flag, u32 stage_index, VulkanShaderStage* shader_stages);