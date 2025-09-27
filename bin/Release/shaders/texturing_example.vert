#version 460 core

layout(location=0) in vec4 VERT_POS;
layout(location=1) in vec4 VERT_COLOUR;
layout(location=5) in vec2 VERT_TEX_COORD;

out vec2 TEX_COORD;
out vec4 VERTEX_COLOUR;


void main()
{
    gl_Position = VERT_POS;
    VERTEX_COLOUR = vec4(VERT_COLOUR.rg + VERT_TEX_COORD, VERT_COLOUR.b + VERT_TEX_COORD.g,1) ;
    TEX_COORD = VERT_TEX_COORD;
}