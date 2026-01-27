#version 450

layout(set = 2, binding = 0) uniform Camera {
    mat4 view;
    mat4 proj;
} camera;

struct ObjectData {
    mat4 model;
    uint textureIndex;
    uint padding[3];
};

layout(push_constant) uniform PushConstants {
    uint objectId;
};

layout(std430, set = 1, binding = 0) readonly buffer ObjectBuffer {
    ObjectData objects[];
};

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 UV;
layout(location = 0) out vec4 fragColor;
layout(location = 1) out uint textureIndex;
layout(location = 2) out vec2 vUV;

void main() {
    gl_Position = camera.proj * camera.view * objects[objectId].model * vec4(position, 1.0);
    fragColor = color;
    textureIndex = objects[objectId].textureIndex;
    vUV = UV;
}
