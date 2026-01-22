#version 450

layout(set = 0, binding = 0) uniform sampler2D offscreen;

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;

void main() {
    vec4 color = texture(offscreen, uv);
    outColor = vec4(color.rgb, 1.0);
}