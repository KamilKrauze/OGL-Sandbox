#version 460 core

layout(location=0) in vec3 VERT_POS;
layout(location=1) in vec4 VERT_COLOUR;
layout(location=2) in vec3 VERT_NORMAL;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform mat3 normal_matrix;

out vec4 FRAG_COLOUR;
out vec3 FRAG_NORMAL;

out vec3 PIXEL_POSITION;
out vec3 LIGHT_POS;

const mat4 modelView_matrix = view * model;

void main()
{
    const vec4 POSITION_H = vec4(VERT_POS, 1);
    FRAG_COLOUR = VERT_COLOUR;
    FRAG_NORMAL = normal_matrix * VERT_NORMAL;
    
    PIXEL_POSITION = vec3(view * model * POSITION_H);
    gl_Position = projection * view * model * POSITION_H;
}
