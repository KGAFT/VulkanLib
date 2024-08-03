#version 460
#extension GL_EXT_ray_tracing: require

#include "RTBase.glsl"

layout (location = 0) rayPayloadInEXT vec3 hitValue;

layout (push_constant) uniform PushConstantRay {
    // vec4 clearColor;
    mat4 viewMatrix;
    mat4 projectionMatrix;
} pcRay;

void main()
{
    hitValue = vec3(0, 0, 0);
}