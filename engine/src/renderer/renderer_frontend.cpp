#include "renderer_frontend.h"
#include "renderer_backend.h"
#include "core/logger.h"
#include "core/dmemory.h"
#include "math/dmath.h"

struct RendererSystemState {
    RendererBackend backend;
    Mat4 projection;
    Mat4 view;
    f32 near_clip;
    f32 far_clip;
};

static RendererSystemState* renderer_state_ptr;

b8 RendererSystemInitialize(u64* memory_requirement, void* state, char* application_name) {
    *memory_requirement = sizeof(RendererSystemState);
    if (state == 0) {
        return true;
    }
    renderer_state_ptr = (RendererSystemState*)state;

    RendererBackendCreate(RENDERER_BACKEND_TYPE_VULKAN, &renderer_state_ptr->backend);
    renderer_state_ptr->backend.frame_number = 0;

    if (!renderer_state_ptr->backend.Initialize(&renderer_state_ptr->backend, application_name)) {
        DFATAL("Renderer backend failed to initialize. Shutting down.");
        return false;
    }

    renderer_state_ptr->near_clip = 0.1f;
    renderer_state_ptr->far_clip = 1000.0f;
    renderer_state_ptr->projection = Mat4Perspective(DegToRad(45.0f), 1280.0f/720.0f, renderer_state_ptr->near_clip, renderer_state_ptr->far_clip);
    renderer_state_ptr->view = Mat4Translation({0, 0, -30.0f});
    renderer_state_ptr->view = Mat4Inverse(renderer_state_ptr->view);
    return true;
}

void RendererSystemShutdown(void* state) {
    if (renderer_state_ptr) {
        renderer_state_ptr->backend.Shutdown(&renderer_state_ptr->backend);
    }
    renderer_state_ptr = 0;
}

b8 RendererBeginFrame(f32 delta_time) {
    if (!renderer_state_ptr) {
        return false;
    }
    return renderer_state_ptr->backend.BeginFrame(&renderer_state_ptr->backend, delta_time);
}

b8 RendererEndFrame(f32 delta_time) {
    if (!renderer_state_ptr) {
        return false;
    }
    b8 result = renderer_state_ptr->backend.EndFrame(&renderer_state_ptr->backend, delta_time);
    renderer_state_ptr->backend.frame_number++;
    return result;
}

void RendererOnResized(u16 width, u16 height) {
    if (renderer_state_ptr) {
        renderer_state_ptr->projection = Mat4Perspective(DegToRad(45.0f), (f32)width/(f32)height, renderer_state_ptr->near_clip, renderer_state_ptr->far_clip);
        renderer_state_ptr->backend.Resized(&renderer_state_ptr->backend, width, height);
    } else {
        DWARN("Renderer Backend does not exist to accept resize: %i, %i", width, height);
    }
}

b8 RendererDrawFrame(RenderPacket* packet) {
    if (RendererBeginFrame(packet->delta_time)) {
        renderer_state_ptr->backend.UpdateGlobalState(renderer_state_ptr->projection, renderer_state_ptr->view, {}, Vec4One(), 0);

        static f32 angle = 0.01f;
        angle += 0.001f;
        Quat rotation = QuatFromAxisAngle(Vec3Forward(), angle, false);
        Mat4 model = QuatToRotationMatrix(rotation, {});
        renderer_state_ptr->backend.UpdateObject(model);

        b8 result = RendererEndFrame(packet->delta_time);
        if (!result) {
            DERROR("RendererEndFrame failed. Application shutting down.");
            return false;
        }
    }
    return true;
}

void RendererSetView(Mat4 view) {
    renderer_state_ptr->view = view;
}