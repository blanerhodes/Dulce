#include "game.h"
#include <entry.h>
#include <core/dmemory.h>

b8 CreateGame(Game* outGame){
    outGame->appConfig.startPosX = 100;
    outGame->appConfig.startPosY = 100;
    outGame->appConfig.startWidth = 1280;
    outGame->appConfig.startHeight = 720;
    outGame->appConfig.name = "Dulce Engine Testbed";
    outGame->Update = GameUpdate;
    outGame->Render = GameRender;
    outGame->Initialize = GameInitialize;
    outGame->OnResize = GameOnResize;
    outGame->state = DAllocate(sizeof(GameState), MEMORY_TAG_GAME);
    outGame->applicationState = 0;
    return true;
}
