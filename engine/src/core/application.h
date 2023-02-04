#pragma once

#include "defines.h"

struct Game;

struct ApplicationConfig{
    i16 startPosX;
    i16 startPosY;
    i16 startWidth;
    i16 startHeight;
    char* name;
};

DAPI b8 ApplicationCreate(Game* gameInst);
DAPI b8 ApplicationRun();