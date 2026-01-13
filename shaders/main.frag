#version 450

layout(set = 1, binding = 0) uniform sampler2D textures[16];

layout(location = 0) in vec4 fragColor;
layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(textures[]) fragColor;
}
