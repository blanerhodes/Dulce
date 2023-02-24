#pragma once

#include "core/application.h"
#include "core/logger.h"
#include "game_types.h"

extern b8 CreateGame(Game* outGame);

int main(void){


    Game gameInst = {};
    if(!CreateGame(&gameInst)){
        DFATAL("Could not create game!");
        return -1;
    }

    if(!gameInst.Render || !gameInst.Update || !gameInst.Initialize || !gameInst.OnResize){
        DFATAL("The game's function pointers must be assigned!");
    }

    if(!ApplicationCreate(&gameInst)){
        DINFO("Application failed to create!\n");
        return 1;
    }
    if(!ApplicationRun()){
        DINFO("Application did not shutdown gracefully");
        return 2;
    }

    return 0;
}