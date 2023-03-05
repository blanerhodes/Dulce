#pragma once

#include "defines.h"
#include "math/math_types.h"

struct Texture {
    u32 id;
    u32 width;
    u32 height;
    u32 generation;
    u8 channel_count;
    b8 has_transparency;
    void* internal_data;
};