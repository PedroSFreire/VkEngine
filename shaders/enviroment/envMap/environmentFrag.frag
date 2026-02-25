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




void main() {


	vec3 color = texture(skyboxTexture,  normalize(vec3(inPosition.x,-inPosition.y,inPosition.z))).rgb ;
  
    outColor.xyz = color;
    outColor.w =1;
}