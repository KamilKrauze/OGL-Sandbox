#version 460 core

layout (location = 0) out vec4 TransmissionMap;

uniform vec4 TranslucentColour;

void main()
{
    if (TranslucentColour.a >= 1.0f)
    {
        TransmissionMap.rgb = vec3(0.0f);
        TransmissionMap.a = 1.0f;
    }
    else {
        TransmissionMap = TranslucentColour;
    }
}