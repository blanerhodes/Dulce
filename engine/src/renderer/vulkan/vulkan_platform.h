#pragma once

#include "defines.h"

struct VulkanContext;

b8 PlatformCreateVulkanSurface(VulkanContext* context);

void PlatformGetRequiredExtensionNames(char*** namesDarray);