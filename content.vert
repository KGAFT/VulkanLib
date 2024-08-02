#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uvs;

layout(location = 0) out vec2 ReadyUvs;

#include <testinc.glsl>

void main() {
    ReadyUvs = uvs;
    gl_Position = vec4(position, 1.0);
}