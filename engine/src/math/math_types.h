#pragma once

#include "defines.h"

union Vec2{
    f32 e[2];
    struct{
        union{
            f32 x, r, s, u;
        };
        union{
            f32 y, g, t, v;
        };
    };
};

union Vec3{
    f32 e[3];
    struct{
        union{
            f32 x, r, s, u;
        };
        union{
            f32 y, g, t, v;
        };
        union{
            f32 z, b, p, w;
        };
    };
};

union Vec4{
#if defined(DUSE_SIMD)
    alignas(16) __m128 data;
#endif
    alignas(16) f32 e[4];
    struct{
        union{
            f32 x, r, s;
        };
        union{
            f32 y, g, t;
        };
        union{
            f32 z, b, p;
        };
        union{
            f32 w, a, q;
        };
    };
};


union Quat{
#if defined(DUSE_SIMD)
    alignas(16) __m128 data;
#endif
    alignas(16) f32 e[4];
    struct{
        union{
            f32 x, r, s;
        };
        union{
            f32 y, g, t;
        };
        union{
            f32 z, b, p;
        };
        union{
            f32 w, a, q;
        };
    };
};

union Mat4{
    alignas(16) f32 data[16];
#if defined(DUSE_SIMD)
    alignas(16) Vec4 rows[4];
#endif
};
