#version 450

in vec4 vertColour;
out vec4 fragColour;

void main()
{
//    fragColour = vec4(1,0,0,1);
    fragColour = vertColour;
}