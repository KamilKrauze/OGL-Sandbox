#version 460 core
layout (location = 0) in vec3 VERT_POSITION;

uniform mat4 LightSpaceMatrix;
uniform mat4 model;

void main()
{
    gl_Position = LightSpaceMatrix * model * vec4(VERT_POSITION, 1.0);
}