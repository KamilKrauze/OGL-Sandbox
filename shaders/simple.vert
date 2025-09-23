#version 460 core

layout(location=0) in vec3 VERT_POS;
layout(location=1) in vec3 VERT_COLOUR;
out vec4 vertColour;

struct SSBOLayout1 {
    vec4 pos;
    vec4 color;
};

uniform vec3 uPosition;
layout (binding = 0, std430) readonly buffer SSBO1
{
    SSBOLayout1 data[];
};

void main()
{
    gl_Position = vec4(VERT_POS + data[gl_InstanceID].pos.xyz,1);
    vertColour = vec4(data[gl_InstanceID].color.rgb,1);
}