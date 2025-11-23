#version 460
layout(location = 0) in vec2 uvs;


layout(set = 0, binding = 0) uniform sampler2D firstLayer;
layout(set = 0, binding = 1) uniform sampler2D secondLayer;
layout(set = 0, binding = 2) uniform sampler2D thirdLayer;

layout(location = 0) out vec4 FragColor;

layout(push_constant) uniform OutputConfig{

    int enableInput[3];
    int dep;

} config;

void main() {
    vec4 resColor = vec4(0);
    if(config.enableInput[0]>=1){
        resColor+=texture(firstLayer, uvs);
    }
    if(config.enableInput[1]>=1){
        resColor+=texture(secondLayer, uvs);
    }
    if(config.enableInput[2]>=1){
        resColor+=texture(thirdLayer, uvs);
    }
    FragColor = resColor;
}