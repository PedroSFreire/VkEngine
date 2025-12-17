#version 450


layout(set = 0, binding = 0) uniform UniformBufferObject {
     mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

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
>>>>>>> ba1e340 (fast GLTF is implemented material data is already in gpu just missing the lights .)
}pc;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * pc.transform * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}