#version 450



struct LightGPUData {
	uint type;
	 uint paddingfat[3];      

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
    mat4 view;
    mat4 proj;
    vec3 cameraPos;
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
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec4 inTangent;

layout(location = 0) out vec4 outColor;

void main() {


	//calculate TBN matrix
	vec3 T = normalize( inTangent.xyz);
	vec3 N = normalize( inNormal);
	vec3 B = cross(N, T) * inTangent.w;
    mat3 TBN = mat3(T, B, N);
	
	//calculate tan space normal
	vec3 tangentNormal = texture(normalTexture, fragTexCoord).rgb;
	//tangentNormal.g *= -1;
	tangentNormal = normalize(tangentNormal * 2.0 - 1.0);
	
	vec3 fNormal = normalize(TBN * tangentNormal);
	
	//Dir vector Calculations
	//temp cpu hardcoded light

	vec3 lightDir = normalize((lights[0].position) - inPosition);

	vec3 viewDir = normalize( ubo.cameraPos - inPosition );
	vec3 halfwayVec = normalize(lightDir + viewDir);
	
	//temp phong
	float spec = pow(max(dot(fNormal, halfwayVec), 0.0), 16.0);
	float metallic = float(pc.metallicFactor) / 255.0; 
	vec3 specular = 0.5 * spec * lights[0].color; 
	//
	float diff = max(dot(fNormal, lightDir), 0.0);
	vec3 diffuse = diff * lights[0].color * lights[0].intensity;

	outColor = vec4((  vec3(0.1) + diffuse) * texture(ColorTexture, fragTexCoord).rgb  + specular  ,1.0) ;





}