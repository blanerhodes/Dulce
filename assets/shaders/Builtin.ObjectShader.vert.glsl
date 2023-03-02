#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 in_position;

layout (set = 0, binding = 0) uniform GlobalUniformObject {
    mat4 projection;
    mat4 view;
} global_ubo;

layout (push_constant) uniform PushConstants {
    //only guaranteed to be minimum 128 bytes
    mat4 model;
} u_push_constants;

void main() {
    gl_Position = global_ubo.projection * global_ubo.view * u_push_constants.model * vec4(in_position, 1.0f);
}