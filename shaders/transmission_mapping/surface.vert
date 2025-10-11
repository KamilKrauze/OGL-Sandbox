#version 460 core

layout(location=0) in vec3 VERT_POS;
layout(location=1) in vec4 VERT_COLOUR;
layout(location=2) in vec3 VERT_NORMAL;
layout(location=5) in vec2 VTEX_COORDS;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform mat3 normal_matrix;
uniform mat4 LightSpaceMatrix;

out vec4 FRAG_COLOUR;
out vec3 FRAG_NORMAL;
out vec2 TexCoords;

out vec4 PIXEL_POSITION;
out vec3 LIGHT_POS;
out vec4 PIXEL_POSITION_LIGHT_SPACE;

const mat4 modelView_matrix = view * model;

void main()
{
    const vec4 POSITION_H = vec4(VERT_POS, 1);
    FRAG_COLOUR = VERT_COLOUR;
    FRAG_NORMAL = normal_matrix * VERT_NORMAL;
    
    PIXEL_POSITION = view * model * POSITION_H;
    PIXEL_POSITION_LIGHT_SPACE = LightSpaceMatrix * (model * POSITION_H);
    TexCoords = VTEX_COORDS;
    gl_Position = projection * view * model * POSITION_H;
}
