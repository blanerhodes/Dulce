#pragma once

#include "core/application.h"

struct Game{
    ApplicationConfig appConfig;
    b8 (*Initialize)(Game* gameInst);
    b8 (*Update)(Game* gameInst, f32 deltatTime);
    b8 (*Render)(Game* gameInst, f32 deltaTime);
    void (*OnResize)(Game* gameInst, u32 width, u32 height);
    void* state;
};