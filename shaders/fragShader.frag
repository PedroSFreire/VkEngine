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
	uint padding0[3];
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



vec3 calcDirectLight(vec3 fNormal, uint i)
{
	//Calculate directions
	vec3 viewDir = normalize( ubo.cameraPos - inPosition );
	vec3 halfwayVec = normalize(lights[i].direction + viewDir);
	
	//specular
	float spec = pow(max(dot(fNormal, halfwayVec), 0.0), 16.0);
	float metallic = float(pc.metallicFactor) / 255.0; 
	vec3 specular = 0.5 * spec * lights[i].color * lights[i].intensity;


	//diffuse
	float diff = max(dot(fNormal, lights[i].direction), 0.0);
	vec3 diffuse = diff * lights[i].color * lights[i].intensity;

	vec3 contrib = diffuse * texture(ColorTexture, fragTexCoord).rgb  +specular;
	return contrib;
}

vec3 calcPointLight(vec3 fNormal, uint i)
{
	//Calculate directions
	vec3 lightDir = normalize((lights[i].position) - inPosition);
	vec3 viewDir = normalize( ubo.cameraPos - inPosition );
	vec3 halfwayVec = normalize(lightDir + viewDir);
	
	//specular
	float spec = pow(max(dot(fNormal, halfwayVec), 0.0), 16.0);
	float metallic = float(pc.metallicFactor) / 255.0; 
	vec3 specular = 0.5 * spec * lights[i].color * lights[i].intensity;


	//diffuse
	float diff = max(dot(fNormal, lightDir), 0.0);
	vec3 diffuse = diff * lights[i].color * lights[i].intensity;

	vec3 contrib = diffuse * texture(ColorTexture, fragTexCoord).rgb  +specular;
	return contrib;

}

vec3 calcSpotLight(vec3 fNormal, uint i)
{
	//Calculate theta
	vec3 lightDir = normalize((lights[i].position) - inPosition);
	float theta = dot( normalize(lights[i].direction),normalize(-lightDir));

	vec3 contrib;


	if(theta > lights[i].spotOuterCos){
		//Calculate directions
		vec3 viewDir = normalize( ubo.cameraPos - inPosition );
		vec3 halfwayVec = normalize(lightDir + viewDir);

		//Calculate intensity
		float intensity;
		if(theta > lights[i].spotInnerCos)
			intensity = lights[i].intensity;
		else intensity = clamp((theta - lights[i].spotOuterCos) / (lights[i].spotInnerCos - lights[i].spotOuterCos), 0.0, 1.0)* lights[i].intensity;

		//specular
		float spec = pow(max(dot(fNormal, halfwayVec), 0.0), 16.0);
		float metallic = float(pc.metallicFactor) / 255.0;
		vec3 specular = 0.5 * spec * lights[i].color * intensity;
		
		//diffuse	
		float diff = max(dot(fNormal, lightDir ), 0.0);
		vec3 diffuse = diff * lights[i].color * intensity;
		contrib = diffuse * texture(ColorTexture, fragTexCoord).rgb  +specular;

	}
	else  contrib = vec3(0.0,0.0,0.0);

	return contrib;
}




void main() {


	//calculate TBN matrix
	vec3 T = normalize( inTangent.xyz);
	vec3 N = normalize( inNormal);
	T = normalize(T - N * dot(N, T));
	vec3 B = cross(N, T) * inTangent.w;

    mat3 TBN = mat3(T, B, N);
	
	//calculate tan space normal
	vec3 tangentNormal = texture(normalTexture, fragTexCoord).rgb;
	tangentNormal = normalize(tangentNormal * 2.0 - 1.0);
	
	vec3 fNormal = normalize(TBN * tangentNormal);
	
	outColor = vec4(0.0,0.0,0.0,1.0);
	for(uint i = 0; i<ubo.numLights;i++){

		if(lights[i].type == 0){
			outColor.xyz += calcDirectLight(fNormal,i);
		}
		else if(lights[i].type == 1){
			outColor.xyz += calcPointLight(fNormal, i);
		}
		else if(lights[i].type == 2){
			outColor.xyz += calcSpotLight(fNormal, i);
		}
	}




}