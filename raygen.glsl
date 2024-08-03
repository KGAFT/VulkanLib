#version 460
#extension GL_EXT_ray_tracing: require
#include "RTBase.glsl"

struct hitPayload
{
    vec3 hitValue;

};



layout (location = 0) rayPayloadEXT hitPayload prd;

layout (push_constant) uniform PushConstantRay {
    mat4 viewMatrix;
    mat4 projectionMatrix;
} pcRay;

void main()
{
    const vec2 pixelCenter = vec2(gl_LaunchIDEXT.xy) + vec2(0.5);
    const vec2 inUV = pixelCenter / vec2(gl_LaunchSizeEXT.xy);
    vec2 d = inUV * 2.0 - 1.0;
    vec4 origin = pcRay.viewMatrix * vec4(0, 0, 0, 1);
    vec4 target = pcRay.projectionMatrix * vec4(d.x, d.y, 1, 1);
    vec4 direction = pcRay.viewMatrix * vec4(normalize(target.xyz), 0);
    uint rayFlags = gl_RayFlagsOpaqueEXT;
    float tMin = 0.001;
    float tMax = 10000000000.0;

    traceRayEXT(topLevelAS,
                rayFlags,
                0xFF,
                0,
                0,
                0,
                origin.xyz,
                tMin,
                direction.xyz,
                tMax,
                0
    );
    imageStore(image, ivec2(gl_LaunchIDEXT.xy), vec4(prd.hitValue, 1.0f));
}