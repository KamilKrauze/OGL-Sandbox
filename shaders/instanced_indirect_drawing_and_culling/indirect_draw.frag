#version 460 core

in vec3 FRAG_NORMAL;

out vec4 FragColour;

void main()
{
    FragColour = vec4(FRAG_NORMAL, 1.0);
}