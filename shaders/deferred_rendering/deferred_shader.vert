#version 460 core
layout(location=0) in vec3 VERT_POS;
layout(location=1) in vec4 VERT_COLOUR;
layout(location=2) in vec3 VERT_NORMAL;
layout(location=5) in vec2 VTEX_COORDS;

out vec2 TexCoords;
out vec3 light_pos;

void main()
{
    light_pos = vec3(0,0.75,1);
    TexCoords = VTEX_COORDS;
    gl_Position = vec4(VERT_POS, 1);
}