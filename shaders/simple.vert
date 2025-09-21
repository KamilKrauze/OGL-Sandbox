#version 450

layout(location=0) in vec3 VERT_POS;
layout(location=1) in vec3 VERT_COLOUR;

out vec4 vertColour;

void main()
{
    float finstanceID = float(gl_InstanceID);
    gl_Position = vec4(VERT_POS + vec3(0, finstanceID/1.5, 0),1);
    vertColour = vec4(VERT_COLOUR / (min(finstanceID,1.0) + 1),1);
}