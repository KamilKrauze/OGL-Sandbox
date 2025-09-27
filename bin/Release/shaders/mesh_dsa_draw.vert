#version 460 core

layout(location=0) in vec3 VERT_POS;
layout(location=1) in vec4 VERT_COLOUR;
layout(location=2) in vec3 VERT_NORMAL;

out vec3 FRAG_NORMAL;
out vec4 FRAG_COLOUR;

void main()
{
    gl_Position = vec4(VERT_POS, 1);
    FRAG_COLOUR = VERT_COLOUR;
    FRAG_NORMAL = VERT_NORMAL;
}
