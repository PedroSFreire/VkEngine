#version 450

layout(location = 0) in vec3 pos;
layout(location = 0) out vec4 FragColor;

layout(push_constant) uniform Push {
    mat4 view;
    mat4 proj;
    float roughness;
    float padding[3];
} pc;

layout(set = 0, binding = 0) uniform samplerCube  envMap;


#include "../../include/constants.glsl"
#include "../../include/brdf/samplingHelpers.glsl"



void main()
{		

   vec3 N = normalize(pos);    
    vec3 R = N;
    vec3 V = R;

    const uint SAMPLE_COUNT = 1024u;
    float totalWeight = 0.0;   
    vec3 prefilteredColor = vec3(0.0);     
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H  = ImportanceSampleGGX(Xi, N, pc.roughness);
        vec3 L  = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);
        if(NdotL > 0.0)
        {
            prefilteredColor += texture(envMap, L).rgb * NdotL;
            totalWeight      += NdotL;
        }
    }
    prefilteredColor = prefilteredColor / totalWeight;

    FragColor = vec4(prefilteredColor, 1.0);
}