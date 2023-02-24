#pragma once
#include "vulkan_types.inl"

char* VulkanResultString(VkResult result, b8 getExtended);

b8 VulkanResultIsSuccess(VkResult result);