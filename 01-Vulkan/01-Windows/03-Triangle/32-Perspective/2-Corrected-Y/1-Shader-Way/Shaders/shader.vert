#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 vPosition;
layout(binding = 0) uniform mvpMatrix
    {
        mat4 modelMatrix;
        mat4 viewMatrix;
        mat4 projectionMatrix;
    } uMvp;

void main(void)
{
    // Code
    gl_Position = uMvp.projectionMatrix * uMvp.viewMatrix * uMvp.modelMatrix * vPosition;

    // flip the Y- co-ordinate to correct y-axis
    gl_Position.y = -gl_Position.y;
}