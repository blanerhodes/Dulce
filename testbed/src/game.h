#pragma once

#include <defines.h>
#include <game_types.h>
#include <math/math_types.h>

struct GameState{
    f32 delta_time;
    //hacky hack
    Mat4 view;
    Vec3 camera_position;
    Vec3 camera_euler;
    b8 camera_view_dirty;
};

b8 GameInitialize(Game* game_inst);
b8 GameUpdate(Game* game_inst, f32 delta_time);
b8 GameRender(Game* game_inst, f32 delta_time);
void GameOnResize(Game* game_inst, u32 width, u32 height);