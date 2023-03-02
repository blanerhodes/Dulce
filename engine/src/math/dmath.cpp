#include "dmath.h"
#include "platform/platform.h"

#include <math.h>
#include <stdlib.h>

static b8 randSeeded = false;

f32 dsin(f32 x){
    return sinf(x);
}

f32 dcos(f32 x){
    return cosf(x);
}

f32 dtan(f32 x){
    return tanf(x);
}

f32 dacos(f32 x){
    return acosf(x);
}

f32 dsqrt(f32 x){
    return sqrtf(x);
}

f32 dabs(f32 x){
    return fabsf(x);
}

i32 drandom(){
    if(!randSeeded){
        srand((u32)PlatformGetAbsoluteTime());
        randSeeded = true;
    }
    return rand();
}

i32 drandomInRange(i32 min, i32 max){
    if(!randSeeded){
        srand((u32)PlatformGetAbsoluteTime());
        randSeeded = true;
    }
    return (rand() % (max - min + 1)) + min;
}

f32 drandomF(){
    return (f32)drandom() / (f32)RAND_MAX;
}

f32 drandomFInRange(f32 min, f32 max){
    return min + ((f32)drandom() / ((f32)RAND_MAX / (max - min)));
}