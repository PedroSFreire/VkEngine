#version 450


layout(set = 0, binding = 0) uniform UniformBufferObject {
     mat4 model;
    mat4 view;
    mat4 proj;
    uint numLights;
} ubo;

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

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 outPosition;

void main() {
    outPosition = inPosition;
    gl_Position = ubo.proj * ubo.view * ubo.model * pc.transform * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}