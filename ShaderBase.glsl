#extension GL_EXT_shader_explicit_arithmetic_types : require

struct Material {
    uint albedoIndex;
    uint normalMapIndex;
    uint metallicMapIndex;
    uint roughnessMapIndex;
    uint aoMapIndex;
    uint emissiveMapIndex;
    uint opacityMapIndex;
};



layout (binding = 0, set = 0) uniform sampler2D textures[];

layout (std430, binding = 1, set = 0) buffer MaterialBuffer {
    Material materials[];
} materialBuffer;