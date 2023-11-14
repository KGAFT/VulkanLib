#version 450

layout(location = 0) in vec2 uvs;


layout(set = 0, binding = 1) uniform sampler2D GameLayer;

layout(location = 0) out vec4 FragColor;

layout(push_constant) uniform ColorCorrect{
    vec4 colorAmplifier;
    vec4 colorAdder;
} correct;

void main() {
    FragColor = texture(GameLayer, uvs)*correct.colorAmplifier+correct.colorAdder;
}
