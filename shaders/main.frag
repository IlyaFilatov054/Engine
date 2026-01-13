#version 450

layout(set = 2, binding = 0) uniform sampler2D textures[16];

layout(location = 0) in vec4 fragColor;
layout(location = 1) flat in uint textureIndex;
layout(location = 2) in vec2 vUV;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(textures[textureIndex], vUV) * fragColor;
}
