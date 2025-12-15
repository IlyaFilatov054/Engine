#version 450

layout(set = 0, binding = 0) uniform Camera {
    mat4 view;
    mat4 proj;
} camera;

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 0) out vec4 fragColor;

void main() {
    gl_Position = camera.proj * camera.view * vec4(position, 1.0);
    fragColor = color;
}
