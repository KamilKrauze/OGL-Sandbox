#version 460 core

layout(location=0) in vec3 VERT_POS;
layout(location=5) in vec2 TEX_COORD;

out vec2 FS_TEX_COORD;

void main()
{
    FS_TEX_COORD = TEX_COORD;
    
    gl_Position = vec4(VERT_POS,1);
}