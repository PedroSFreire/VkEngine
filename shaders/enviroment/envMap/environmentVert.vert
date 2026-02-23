#version 450


layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec3 cameraPos;
    uint numLights;
} ubo;



layout(location = 0) in vec3 inPosition;


layout(location = 0) out vec3 outPos;


void main() {

  // Remove translation from view
    mat4 viewRot = mat4(mat3(ubo.view));

    outPos = inPosition;

    gl_Position = ubo.proj * viewRot * vec4(inPosition, 1.0);

  
    //gl_Position.z = gl_Position.w;

}