#version 460 core

layout(location=0) in vec3 VERT_POS;
layout(location=1) in vec4 VERT_COLOUR;
layout(location=2) in vec3 VERT_NORMAL;

struct Object
{
    mat4 model;
    vec4 sphere;
};

layout(std430, binding = 0) readonly buffer Objects
{
    Object objects[];
};

uniform mat4 viewProjection;

out vec3 FRAG_NORMAL;

void main()
{
    // baseInstance identifies which object produced this draw.
    uint objectID = gl_BaseInstance;

    gl_Position =
    viewProjection *
    objects[objectID].model *
    vec4(VERT_POS, 1.0);

    FRAG_NORMAL = VERT_NORMAL;
}