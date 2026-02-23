#version 450

layout (location = 0) in vec3 aPos;


layout(location = 0) out vec3 vDir;

layout(push_constant) uniform Push {
    mat4 view;
    mat4 proj;
} pc;

void main()
{

    mat3 viewRot = mat3(pc.view);


    vDir =  aPos;


    gl_Position = pc.proj *mat4(viewRot)* vec4(aPos, 1.0);

}