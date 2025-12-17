#version 450

layout(set = 1, binding = 0) uniform sampler2D  ColorTexture;
layout(set = 1, binding = 1) uniform sampler2D  normalTexture;
layout(set = 1, binding = 2) uniform sampler2D  metalRoughTexture;
layout(set = 1, binding = 3) uniform sampler2D  occlusionTexture;
layout(set = 1, binding = 4) uniform sampler2D  emissiveTexture;


layout(push_constant) uniform PushConstants{
    mat4 transform;
    vec4 colorFactor;
<<<<<<< HEAD

    uint metallicFactor;
	uint roughnessFactor;
	uint emissiveStrenght;
    uint padding0;

	vec3 emissiveFactor;
    uint padding1;
=======
    uint metallicFactor;
	uint roughnessFactor;
	uint emissiveStrenght;
	vec3 emissiveFactor;
>>>>>>> ba1e340 (fast GLTF is implemented material data is already in gpu just missing the lights .)
}pc;


layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;


layout(location = 0) out vec4 outColor;

void main() {
    outColor = pc.colorFactor*texture(ColorTexture,fragTexCoord);

}