#version 460


layout(location = 0) out vec4 FragColor;

layout(push_constant) uniform OutputConfig{

    int enableInput[3];
    int dep;

} config;

void main() {
    vec4 resColor = vec4(1.0, 0.0, 0.0, 1.0);
    FragColor = resColor;
}