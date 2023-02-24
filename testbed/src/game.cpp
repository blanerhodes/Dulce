#include "game.h"
#include <core/logger.h>
#include <core/input.h>
#include <core/dmemory.h>

b8 GameInitialize(Game* gameInst){
    DDEBUG("GameInitialize() called");
    return true;
}

b8 GameUpdate(Game* gameInst, f32 deltaTime){
    static u64 allocCount = 0;
    u64 prevAllocCount = allocCount;
    allocCount = GetMemoryAllocCount();

    if(InputIsKeyUp(KEY_M) && InputWasKeyDown(KEY_M)){
        DDEBUG("Allocations: %llu (%llu this frame)", allocCount, allocCount - prevAllocCount);
    }
    return true;
}

b8 GameRender(Game* gameInst, f32 deltaTime){
    return true;
}

void GameOnResize(Game* gameInst, u32 width, u32 height){
}