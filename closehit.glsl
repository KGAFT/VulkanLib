#version 460
#extension GL_EXT_ray_tracing: require
#extension GL_EXT_nonuniform_qualifier: enable
#include "RTBase.glsl"

layout (location = 0) rayPayloadInEXT vec3 hitValue;
hitAttributeEXT vec2 attribs;


layout (push_constant) uniform PushConstantRay {
    mat4 viewMatrix;
    mat4 projectionMatrix;
} pcRay;

struct PointLight{
    vec3 position;
    vec3 color;
    float intensity;
};

vec3 getNormalFromMap(vec4 baseNormal, Triangle currentTriangle, vec3 worldNrm)
{
    vec3 tangentNormal = baseNormal.xyz * 2.0 - 1.0;

    vec3 Q1 = currentTriangle.vertices[1].position-currentTriangle.vertices[0].position;
    vec3 Q2 = currentTriangle.vertices[2].position-currentTriangle.vertices[0].position;
    vec2 st1 = currentTriangle.vertices[1].uv-currentTriangle.vertices[0].uv;
    vec2 st2 = currentTriangle.vertices[2].uv-currentTriangle.vertices[0].uv;
    vec3 N = normalize(worldNrm);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);
    return normalize(TBN * tangentNormal);
}

void main()
{
    GeometryNode currentNode = getCurrentGeomtryNode(gl_GeometryIndexEXT);

    Triangle triangle = getCurrentTriangle(currentNode, gl_PrimitiveID, attribs);
    Material material = materialBuffer.materials[triangle.vertices[0].materialIndex];
    vec3 color = texture(textures[nonuniformEXT(material.albedoIndex)], triangle.uv).rgb;
    vec4 normalFromMap = texture(textures[nonuniformEXT(material.normalMapIndex)], triangle.uv);


    const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
    const vec3 pos = triangle.vertices[0].position * barycentrics.x + triangle.vertices[1].position * barycentrics.y + triangle.vertices[2].position * barycentrics.z;
    const vec3 worldPos = vec3(gl_ObjectToWorldEXT * vec4(pos, 1.0));

    const vec3 worldNrm = normalize(vec3(triangle.normal * gl_WorldToObjectEXT));
    vec3 finalNormal = normalize(getNormalFromMap(normalFromMap, triangle, worldNrm));
    hitValue = finalNormal;
}