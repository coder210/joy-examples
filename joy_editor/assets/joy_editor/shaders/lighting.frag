#version 450

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragUV;

layout(set = 2, binding = 0) uniform sampler2D mySampler;

#define MAX_LIGHTS 4
#define LIGHT_DIRECTIONAL 0
#define LIGHT_POINT 1

struct Light {
    int enabled;
    int type;
    vec3 position;
    vec3 target;
    vec4 color;
};

layout(set = 3, binding = 0) uniform LightBlock {
    Light lights[MAX_LIGHTS];
    vec4 ambient;
    vec3 viewPos;
    float _pad;
} lb;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 texelColor = texture(mySampler, fragUV);
    vec3 normal = normalize(fragNormal);
    vec3 viewDir = normalize(lb.viewPos - fragPos);
    vec3 lightAccum = vec3(0.0);
    vec3 specAccum = vec3(0.0);

    for (int i = 0; i < MAX_LIGHTS; i++) {
        if (lb.lights[i].enabled == 1) {
            vec3 lightDir;
            if (lb.lights[i].type == LIGHT_DIRECTIONAL) {
                lightDir = -normalize(lb.lights[i].target - lb.lights[i].position);
            } else {
                lightDir = normalize(lb.lights[i].position - fragPos);
            }
            float NdotL = max(dot(normal, lightDir), 0.0);
            lightAccum += lb.lights[i].color.rgb * NdotL;
            if (NdotL > 0.0) {
                vec3 halfDir = normalize(lightDir + viewDir);
                float spec = pow(max(dot(normal, halfDir), 0.0), 16.0);
                specAccum += lb.lights[i].color.rgb * spec;
            }
        }
    }

    vec3 color = texelColor.rgb * (vec3(0.5) + lightAccum * 0.7) + specAccum * 0.3;
    color += texelColor.rgb * (lb.ambient.rgb / 10.0);
    // gamma
    color = pow(color, vec3(1.0 / 2.2));
    outColor = vec4(color, texelColor.a);
}
