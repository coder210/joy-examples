#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUV;

layout(set = 0, binding = 0) uniform MVPBlock {
    mat4 viewProj;
    mat4 model;
} ubo;

layout(location = 0) out vec2 fragUV;

void main() {
    gl_Position = ubo.viewProj * ubo.model * vec4(inPos, 1.0);
    fragUV = inUV;
}
