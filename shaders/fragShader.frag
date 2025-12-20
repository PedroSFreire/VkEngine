#version 450



struct LightGPUData {
	uint type;

	vec3 color;
	float intensity;

	vec3 position;
	float range;

	vec3 direction;
	float spotInnerCos;

	float spotOuterCos;
	uint padding0;
};

layout(set = 0, binding = 0) uniform UniformBufferObject {
     mat4 model;
    mat4 view;
    mat4 proj;
    uint numLights;
} ubo;


layout(set = 1, binding = 0) uniform sampler2D  ColorTexture;
layout(set = 1, binding = 1) uniform sampler2D  normalTexture;
layout(set = 1, binding = 2) uniform sampler2D  metalRoughTexture;
layout(set = 1, binding = 3) uniform sampler2D  occlusionTexture;
layout(set = 1, binding = 4) uniform sampler2D  emissiveTexture;

layout(set = 2, binding = 0) readonly buffer LightBuffer {
    LightGPUData lights[];
};

layout(push_constant) uniform PushConstants{
    mat4 transform;
    vec4 colorFactor;


    uint metallicFactor;
	uint roughnessFactor;
	uint emissiveStrenght;
    uint padding0;

	vec3 emissiveFactor;
    uint padding1;

}pc;


layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3  inPosition;


layout(location = 0) out vec4 outColor;

void main() {
	vec3 normal =  normalize(texture(normalTexture, fragTexCoord).rgb * 2.0 - 1.0);
	vec3 diffuse = vec3(0.0);
	vec3 lightColor = vec3(0.0);
	for (uint i = 0; i < ubo.numLights; i++) {
	lightColor += lights[i].color * lights[i].intensity;
		vec3 lightDir = normalize(lights[i].position - inPosition); 
		float diff = max(dot(normal, lightDir), 0.0);
		diffuse += diff * lights[i].color;
	}

    //outColor = vec4(diffuse,1.0) * pc.colorFactor*texture(ColorTexture,fragTexCoord);
	outColor = vec4(lightColor,1.0);
	outColor.rgb += pc.emissiveFactor * pc.emissiveStrenght * texture(emissiveTexture, fragTexCoord).r;

}