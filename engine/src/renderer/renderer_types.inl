#pragma once

#include "defines.h"
#include "math/math_types.h"
#include "resources/resource_types.h"

enum RendererBackendType{
    RENDERER_BACKEND_TYPE_VULKAN,
    RENDERER_BACKEND_TYPE_OPENGL,
    RENDERER_BACKEND_TYPE_DIRECTX
};

struct GlobalUniformObject {
    Mat4 projection;
    Mat4 view;
    //padding out to 256
    Mat4 reserved0;
    Mat4 reserved1;
};

struct ObjectUniformObject {
    Vec4 diffuse_color;
    //pad out to 64
    Vec4 reserved0;
    Vec4 reserved1;
    Vec4 reserved2;
};

struct GeometryRenderData {
    u32 object_id;
    Mat4 model;
    Texture* textures[16];
};

struct RendererBackend{
    u32 frame_number;

    b8 (*Initialize)(struct RendererBackend* backend, char* applicationName);

    void (*Shutdown)(struct RendererBackend* backend);

    void (*Resized)(struct RendererBackend* backend, u16 width, u16 height);

    b8 (*BeginFrame)(struct RendererBackend* backend, f32 delta_time);

    void (*UpdateGlobalState)(Mat4 projection, Mat4 view, Vec3 view_position, Vec4 ambient_color, i32 mode);

    b8 (*EndFrame)(struct RendererBackend* backend, f32 delta_time);

    void (*UpdateObject)(GeometryRenderData data);

    void (*CreateTexture)(char* name, b8 auto_release, i32 width, i32 height, i32 channel_count,
                          u8* pixels, b8 has_transparency, Texture* out_texture);
    
    void (*DestroyTexture)(Texture* texture);
};

struct RenderPacket{
    f32 delta_time;
};