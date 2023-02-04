#include "game.h"
#include <core/logger.h>


b8 GameInitialize(Game* gameInst){
    DDEBUG("GameInitialize() called");
    return TRUE;
}

b8 GameUpdate(Game* gameInst, f32 deltaTime){
    return TRUE;
}

b8 GameRender(Game* gameInst, f32 deltaTime){
    return TRUE;
}

void GameOnResize(Game* gameInst, u32 width, u32 height){
}