#version 460 core

in vec4 FRAG_COLOUR;
in vec3 FRAG_NORMAL;
in vec3 PIXEL_POSITION;

uniform float light_intensity;
uniform vec3 LightPos;
uniform vec3 CameraPosition;

out vec4 fragColour;

const float ambient_str = 0.4;
const vec3 random_colour_that_is_deprecated = vec3(0.985, 0.6, 0.5);
vec3 ambient_colour = vec3(0.3) * ambient_str;

vec3 lambert_diffuse(in vec3 albedo, in float intensity, in float dotNL)
{
    const vec3 diffuse = max(dotNL, 0.0f) * albedo * intensity;
    return diffuse;
}

vec3 blinn_phong_specular(in float intensity, in float shininess, in float dotNH)
{
    return pow(max(dotNH, 0.0), shininess) * FRAG_COLOUR.rgb * intensity;
}


void main()
{
    const vec3 N = normalize(FRAG_NORMAL);
    const vec3 L = normalize(LightPos - PIXEL_POSITION);
    const vec3 V = normalize(CameraPosition - PIXEL_POSITION);
    const vec3 R = reflect(-L, N);
    const vec3 H = (normalize(L + V));
    const float dotNL = dot(N,L);
    const float dotNH = dot(N,H);
    
    fragColour.rgb = lambert_diffuse(FRAG_COLOUR.rgb, light_intensity, dotNL) + 
    blinn_phong_specular(light_intensity, 128.0f, dotNH) + ambient_colour;
    
    fragColour.a = 1.0f;
}