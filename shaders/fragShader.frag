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


const float PI = 3.14159265359;

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}  

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 calcDirectLight(vec3 fNormal, uint i)
{
	//get material tex values
	vec3 color = texture(ColorTexture, fragTexCoord).rgb;// * pc.colorFactor.rgb;
	float metallic = texture(metalRoughTexture, fragTexCoord).b;// * pc.metallicFactor;
	float roughness = texture(metalRoughTexture, fragTexCoord).g;// * pc.roughnessFactor;

	//Calculate directions
	vec3 lightDir = normalize(-lights[i].direction);
	vec3 viewDir = normalize( ubo.cameraPos - inPosition );
	vec3 halfwayVec = normalize(lightDir + viewDir);
    vec3 radiance = lights[i].color *  lights[i].intensity; 

	//calculate zero incidence reflection
	vec3 F0 = vec3(0.04); 
	F0 = mix(F0, color, metallic);
	vec3 F  = fresnelSchlick(max(dot(halfwayVec, viewDir), 0.0), F0);

	//specular
	float NDF = DistributionGGX(fNormal, halfwayVec, roughness);       
	float G   = GeometrySmith(fNormal, viewDir, lightDir, roughness); 
	vec3 specular = NDF * G * F / (4.0 * max(dot(fNormal, viewDir), 0.0) * max(dot(fNormal, lightDir), 0.0)  + 0.0001);

	
	//diffuse
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallic;	


	float NdotL = max(dot(fNormal, lightDir), 0.0);        
    vec3 contrib = (kD * color / PI + specular) * radiance * NdotL;
	return contrib;
}

vec3 calcPointLight(vec3 fNormal, uint i)
{

	//get material tex values
	vec3 color = texture(ColorTexture, fragTexCoord).rgb;// * pc.colorFactor.rgb;
	float metallic = texture(metalRoughTexture, fragTexCoord).b;// * pc.metallicFactor;
	float roughness = texture(metalRoughTexture, fragTexCoord).g;// * pc.roughnessFactor;

	//Calculate directions
	vec3 lightDir = normalize((lights[i].position) - inPosition);
	vec3 viewDir = normalize( ubo.cameraPos - inPosition );
	vec3 halfwayVec = normalize(lightDir + viewDir);
	float distance = length(lights[i].position - inPosition);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = lights[i].color * attenuation *  lights[i].intensity; 


	//calculate zero incidence reflection
	vec3 F0 = vec3(0.04); 
	F0 = mix(F0, color, metallic);
	vec3 F  = fresnelSchlick(max(dot(halfwayVec, viewDir), 0.0), F0);

	//specular
	float NDF = DistributionGGX(fNormal, halfwayVec, roughness);       
	float G   = GeometrySmith(fNormal, viewDir, lightDir, roughness); 
	vec3 specular = NDF * G * F / (4.0 * max(dot(fNormal, viewDir), 0.0) * max(dot(fNormal, lightDir), 0.0)  + 0.0001);

	
	//diffuse
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallic;	


	float NdotL = max(dot(fNormal, lightDir), 0.0);        
    vec3 contrib = (kD * color / PI + specular) * radiance * NdotL;
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

		vec3 lightDir = normalize((lights[i].position) - inPosition);
		vec3 viewDir = normalize( ubo.cameraPos - inPosition );
		vec3 halfwayVec = normalize(lightDir + viewDir);
		float distance = length(lights[i].position - inPosition);
		float attenuation = 1.0 / (distance * distance);


		//Calculate intensity
		float intensity;
		if(theta > lights[i].spotInnerCos)
			intensity = lights[i].intensity;
		else intensity = clamp((theta - lights[i].spotOuterCos) / (lights[i].spotInnerCos - lights[i].spotOuterCos), 0.0, 1.0)* lights[i].intensity;


		vec3 radiance = lights[i].color * attenuation * intensity; 

			//get material tex values
		vec3 color = texture(ColorTexture, fragTexCoord).rgb;// * pc.colorFactor.rgb;
		float metallic = texture(metalRoughTexture, fragTexCoord).b;// * pc.metallicFactor;
		float roughness = texture(metalRoughTexture, fragTexCoord).g;// * pc.roughnessFactor;




	//calculate zero incidence reflection
	vec3 F0 = vec3(0.04); 
	F0 = mix(F0, color, metallic);
	vec3 F  = fresnelSchlick(max(dot(halfwayVec, viewDir), 0.0), F0);

	//specular
	float NDF = DistributionGGX(fNormal, halfwayVec, roughness);       
	float G   = GeometrySmith(fNormal, viewDir, lightDir, roughness); 
	vec3 specular = NDF * G * F / (4.0 * max(dot(fNormal, viewDir), 0.0) * max(dot(fNormal, lightDir), 0.0)  + 0.0001);

	
	//diffuse
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallic;	


	float NdotL = max(dot(fNormal, lightDir), 0.0);        
    vec3 contrib = (kD * color / PI + specular) * radiance * NdotL;
	return contrib;

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

	vec3 ambient = vec3(0.1) * texture(ColorTexture,fragTexCoord).rgb * texture(occlusionTexture,fragTexCoord).r;
	outColor.xyz += ambient + texture(emissiveTexture,fragTexCoord).rgb; 



}