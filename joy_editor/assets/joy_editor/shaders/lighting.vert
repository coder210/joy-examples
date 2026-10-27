#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(set = 1, binding = 0) uniform MVPBlock {
    mat4 viewProj;
    mat4 model;
} ubo;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragUV;

void main() {
    vec4 worldPos = ubo.model * vec4(inPos, 1.0);
    fragPos = worldPos.xyz;
    fragNormal = normalize(mat3(ubo.model) * inNormal);
    fragUV = inUV;
    gl_Position = ubo.viewProj * worldPos;
}
