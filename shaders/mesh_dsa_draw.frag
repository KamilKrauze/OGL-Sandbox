#version 460 core

in vec4 FRAG_COLOUR;
in vec3 FRAG_NORMAL;

in vec3 PIXEL_POSITION;
in vec3 LIGHT_POS;

out vec4 fragColour;

const float light_intensity = 1.0f;
const float ambient_str = 0.4;

vec4 ambient_colour = vec4(FRAG_NORMAL,1) * vec4(vec3(0.8), 1) * ambient_str;

vec3 norm_normals;
vec3 norm_lightpos;

vec3 V;
vec3 R;

vec4 diffuse_colour()
{
    vec4 diffuse = max(dot(norm_normals, norm_lightpos), 0.0f) * vec4(FRAG_NORMAL,1) * light_intensity;
    return diffuse;
}

vec4 specular()
{
    return pow(max(dot(R,V), 0.0), 8.0) * light_intensity * vec4(normalize(FRAG_NORMAL), 1);
}

void main()
{
    norm_normals = normalize(FRAG_NORMAL);
    norm_lightpos = normalize(LIGHT_POS);
    
    V = normalize(-PIXEL_POSITION);
    R = reflect(-norm_lightpos, norm_normals);
    
//    fragColour = vec4(FRAG_NORMAL, 1);
    fragColour = diffuse_colour() + specular() + ambient_colour;
}