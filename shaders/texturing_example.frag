#version 450

layout(location=0) in vec3 VERT_POS;

in vec2 TEX_COORD;
in vec4 VERTEX_COLOUR;
out vec4 fragColour;

layout(binding=0) uniform sampler2D tex0;
layout(binding=1) uniform sampler2D tex1;

float saturate(float val)
{
    return clamp(val,0,1);
}

void main()
{
    vec3 point = vec3(0.5, 0.5, 0.5);
    
    const vec3 tex0Colour = texture(tex0, TEX_COORD).rgb;
    const vec3 tex1Colour = texture(tex1, TEX_COORD).rgb;
    
    float alpha = 1-pow(tex0Colour.r, 0.8);
    const vec3 mixed = mix(tex0Colour, tex1Colour, saturate(alpha));
    
    fragColour = vec4(mixed, 1);
    fragColour = vec4(1,1,1,1);
}