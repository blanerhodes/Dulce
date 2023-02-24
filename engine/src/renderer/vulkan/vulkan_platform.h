#pragma once

#include "defines.h"

struct PlatformState;
struct VulkanContext;

b8 PlatformCreateVulkanSurface(PlatformState* platState, VulkanContext* context);

void PlatformGetRequiredExtensionNames(char*** namesDarray);