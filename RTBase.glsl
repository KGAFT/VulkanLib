#extension GL_EXT_nonuniform_qualifier: require
#extension GL_EXT_buffer_reference2: require
#extension GL_EXT_scalar_block_layout: require
#extension GL_EXT_shader_explicit_arithmetic_types_int64: require

#include "ShaderBase.glsl"

layout (binding = 2, set = 0) uniform accelerationStructureEXT topLevelAS;
layout (binding = 3, set = 0, rgba32f) uniform image2D image;

struct UINT64{
    uint firstVal;
    uint secondVal;
};


struct GeometryNode {
    uint32_t materialIndex;
    uint64_t vertexBufferDeviceAddress;
    uint32_t stepSize;
    uint64_t indexBufferDeviceAddress;
};

struct Vertex {
    vec3 position;
    vec3 normal;
    vec2 uv;
    uint materialIndex;
};

struct Triangle {
    Vertex vertices[3];
    vec3 normal;
    vec2 uv;
};

layout (std430, binding = 4, set = 0) buffer GeometryNodesStorage {
    GeometryNode geometryNodes[];
} geometryNodes;

layout (buffer_reference, scalar) buffer FloatArray {float v[]; };
layout (buffer_reference, scalar) buffer Vec4Array {vec4 v[]; };
layout (buffer_reference, scalar) buffer UintArray {uint i[]; };
layout (buffer_reference, scalar) buffer MaterialReference {Material mat[];};

GeometryNode getCurrentGeomtryNode(uint32_t instanceIndex) {

        return geometryNodes.geometryNodes[instanceIndex];

}

Vertex getCurrentVertex(GeometryNode geometryNode, uint32_t primitiveId, uint i) {
    UintArray curIndices = UintArray(geometryNode.indexBufferDeviceAddress);
    FloatArray curVertices = FloatArray(geometryNode.vertexBufferDeviceAddress);
    uint currentIndex = primitiveId * 3;
    const uint offset = curIndices.i[currentIndex+i]*9;
    Vertex result;

    result.position.x = curVertices.v[offset + 0];
    result.position.y = curVertices.v[offset + 1];
    result.position.z = curVertices.v[offset + 2];
    result.normal.x = curVertices.v[offset + 3];
    result.normal.y = curVertices.v[offset + 4];
    result.normal.z = curVertices.v[offset + 5];
    result.uv.x = curVertices.v[offset + 6];
    result.uv.y = curVertices.v[offset + 7];
    result.materialIndex = uint(curVertices.v[offset + 8]);

    return result;
}

Triangle getCurrentTriangle(GeometryNode node, uint32_t primitiveId, vec2 initialUv) {
    Triangle result;
    result.vertices[0] = getCurrentVertex(node, primitiveId, 0);
    result.vertices[1] = getCurrentVertex(node, primitiveId, 1);
    result.vertices[2] = getCurrentVertex(node, primitiveId, 2);
    vec3 barycentricCoords = vec3(1.0f - initialUv.x - initialUv.y, initialUv.x, initialUv.y);
    result.uv = result.vertices[0].uv * barycentricCoords.x + result.vertices[1].uv * barycentricCoords.y + result.vertices[2].uv * barycentricCoords.z;
    result.normal = result.vertices[0].normal * barycentricCoords.x + result.vertices[1].normal * barycentricCoords.y + result.vertices[2].normal * barycentricCoords.z;
    return result;
}