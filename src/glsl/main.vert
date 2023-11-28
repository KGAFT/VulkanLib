#version 450

// vulkan NDC:	x: -1(left), 1(right)
//				y: -1(top), 1(bottom)
layout(location = 0) in vec3 position;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = vec4(position, 1.0);
    fragColor = vec3(position.z, position.y, position.x);
}