#version 450

layout(location=0) in vec3 vertPos;

out vec4 fragColour;

void main()
{
    fragColour = vec4(vertPos,0);
}