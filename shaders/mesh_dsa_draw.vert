#version 460 core

layout(location=0) in vec3 VERT_POS;
layout(location=1) in vec4 VERT_COLOUR;

void main()
{
    gl_Position = vec4(VERT_POS, 1);
}
