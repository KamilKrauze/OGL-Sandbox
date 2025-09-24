#version 460 core

in vec2 FS_TEX_COORD;

out vec4 fragColour;

layout (binding=0) uniform sampler2D tex;
uniform float TIME;
uniform float DELTA_TIME;

void main()
{
    vec3 texColor = texture(tex, FS_TEX_COORD).rgb;
    fragColour = vec4(texColor + vec3(TIME * DELTA_TIME), 1);
//    fragColour = vec4(FS_TEX_COORD,0,1);
}