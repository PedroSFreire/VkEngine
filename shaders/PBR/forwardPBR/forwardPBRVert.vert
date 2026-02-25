#version 450


layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec3 cameraPos;
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
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inTangent;
layout(location = 3) in vec3 inColor;
layout(location = 4) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 outPosition;
layout(location = 3) out vec3 outNormal;
layout(location = 4) out vec4 outTangent;

void main() {
    outPosition = vec3( pc.transform * vec4(inPosition, 1.0));


    mat3 normalMatrix = transpose(inverse( mat3(pc.transform)));
    outNormal = normalize(normalMatrix *normalize(inNormal));

    outTangent.xyz = normalize(normalMatrix *normalize(inTangent.xyz));
    outTangent.w = inTangent.w;

    gl_Position = ubo.proj * ubo.view *  pc.transform * vec4(inPosition, 1.0);

    fragColor = inColor;
    fragTexCoord = inTexCoord;
}