#version 460 core

in vec4 FRAG_COLOUR;
in vec3 FRAG_NORMAL;


out vec4 fragColour;

void main()
{
    fragColour = vec4(FRAG_NORMAL, 1);
}