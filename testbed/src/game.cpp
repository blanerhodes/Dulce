#include "game.h"
#include <core/logger.h>
#include <core/input.h>
#include <core/dmemory.h>
#include <math/dmath.h>

//temp include
#include <renderer/renderer_frontend.h>

void RecalculateViewMatrix(GameState* state) {
    if (state->camera_view_dirty) {
        Mat4 rotation = Mat4EulerXyz(state->camera_euler.x, state->camera_euler.y, state->camera_euler.z);
        Mat4 translation = Mat4Translation(state->camera_position);

        state->view = Mat4Mult(rotation, translation);
        state->view = Mat4Inverse(state->view);
        state->camera_view_dirty = false;
    }
}

void CameraYaw(GameState* state, f32 amount) {
    state->camera_euler.y += amount;
    state->camera_view_dirty = true;
}

void CameraPitch(GameState* state, f32 amount) {
    state->camera_euler.x += amount;
    //prevent gimbal lock
    f32 limit = DegToRad(89.0f);
    state->camera_euler.x = DCLAMP(state->camera_euler.x, -limit, limit);

    state->camera_view_dirty = true;
}

b8 GameInitialize(Game* game_inst) {
    DDEBUG("GameInitialize() called");

    GameState* state = (GameState*)game_inst->state;
    state->camera_position = {0, 0, 30.0f};
    state->camera_euler = {};
    state->view = Mat4Translation(state->camera_position);
    state->view = Mat4Inverse(state->view);
    state->camera_view_dirty = true;
    return true;
}

b8 GameUpdate(Game* game_inst, f32 delta_time) {
    static u64 alloc_count = 0;
    u64 prev_alloc_count = alloc_count;
    alloc_count = GetMemoryAllocCount();

    if (InputIsKeyUp(KEY_M) && InputWasKeyDown(KEY_M)) {
        DDEBUG("Allocations: %llu (%llu this frame)", alloc_count, alloc_count - prev_alloc_count);
    }

    GameState* state = (GameState*)game_inst->state;

    //HACK: camera move
    if (InputIsKeyDown(KEY_A) || InputIsKeyDown(KEY_LEFT)) {
        CameraYaw(state, 1.0f * delta_time);
    }
    if (InputIsKeyDown(KEY_D) || InputIsKeyDown(KEY_RIGHT)) {
        CameraYaw(state, -1.0f * delta_time);
    }
    if (InputIsKeyDown(KEY_UP)) {
        CameraPitch(state, 1.0f * delta_time);
    }
    if (InputIsKeyDown(KEY_DOWN)) {
        CameraPitch(state, -1.0f * delta_time);
    }

    f32 temp_move_speed = 50.0f;
    Vec3 velocity = {};
    if (InputIsKeyDown(KEY_W)) {
        Vec3 forward = Mat4Forward(state->view);
        velocity += forward;
    }
    if (InputIsKeyDown(KEY_S)) {
        Vec3 backward = Mat4Backward(state->view);
        velocity += backward;
    }
    if (InputIsKeyDown(KEY_Q)) {
        Vec3 left = Mat4Left(state->view);
        velocity += left;
    }
    if (InputIsKeyDown(KEY_E)) {
        Vec3 right = Mat4Right(state->view);
        velocity += right;
    }
    if (InputIsKeyDown(KEY_SPACE)) {
        velocity.y += 1.0f;
    }
    if (InputIsKeyDown(KEY_X)) {
        velocity.y -= 1.0f;
    }

    Vec3 z = {};
    if (!Vec3Compare(z, velocity, 0.0002f)) {
        Normalize(&velocity);
        state->camera_position += (velocity * temp_move_speed * delta_time);
        state->camera_view_dirty = true;
    }

    RecalculateViewMatrix(state);

    //temp HACK:
    RendererSetView(state->view);
    return true;
}

b8 GameRender(Game* game_inst, f32 delta_time) {
    return true;
}

void GameOnResize(Game* game_inst, u32 width, u32 height) {
}