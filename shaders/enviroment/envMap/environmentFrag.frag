#version 450



layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec3 cameraPos;
    uint numLights;
} ubo;

layout(set = 1, binding = 0) uniform samplerCube  skyboxTexture;



layout(location = 0) in vec3  inPosition;
layout(location = 0) out vec4 outColor;

vec3 ACESFilm(vec3 x)
{
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
}


void main() {


	vec3 color = texture(skyboxTexture,  normalize(vec3(inPosition.x,-inPosition.y,inPosition.z))).rgb ;
  
    float exposure = 0.4;
    color *= exposure;

    color = ACESFilm(color);
    //color = pow(color, vec3(1.0/2.2));

    outColor = vec4(clamp(color,0,1), 1.0);
}