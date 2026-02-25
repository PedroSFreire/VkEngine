#version 450

layout (location = 0) in vec3 aPos;


layout(location = 0) out vec3 localPos;

layout(push_constant) uniform Push {
    mat4 view;
    mat4 proj;
} pc;

void main()
{
    localPos = aPos;  

    gl_Position =  pc.proj * pc.view * vec4(localPos, 1.0);
}