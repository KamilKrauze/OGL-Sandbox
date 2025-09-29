#version 460 core

in vec2 TexCoords;
in vec3 light_pos;
out vec4 fragColour;

// G-Buffer textures
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D depth;

uniform vec3 CameraPosition;

const float light_intensity = 0.5f;
const float shine_factor = 10.0;

vec3 PIXEL_POSITION;
vec3 PIXEL_NORMALS;
vec3 PIXEL_SPECULAR;
float PIXEL_DEPTH;

vec3 diffuse_colour(float intensity, vec3 L, vec3 N)
{
    float diffuse = max(dot(N, L), 0.0f);
    return vec3(diffuse);
}

vec3 specular(float intensity, float shininess, vec3 L, vec3 N, vec3 H)
{
    return pow(max(dot(N,H), 0.0), shininess) * PIXEL_SPECULAR;
}

float linearize_depth(float depth, float nearPlane, float farPlane)
{
    float z = depth * 2.0 - 1.0;
    float linear_depth = (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));

    return linear_depth;
}        

void main()
{   
    PIXEL_DEPTH = texture(depth, TexCoords).r;
    PIXEL_POSITION = texture(gPosition, TexCoords).rgb;
    PIXEL_NORMALS = texture(gNormal, TexCoords).rgb;
    PIXEL_SPECULAR = texture(gAlbedoSpec, TexCoords).rgb;

    if (PIXEL_DEPTH >= 1.0f)
    {
        discard; return;
    }
    
    vec3 N = normalize(PIXEL_NORMALS);
    vec3 L = normalize(light_pos - PIXEL_POSITION);

    vec3 V = normalize(CameraPosition - PIXEL_POSITION);
    vec3 R = reflect(-PIXEL_POSITION, N);
    vec3 H = (normalize(L + V)); 
    
    fragColour = vec4((diffuse_colour(light_intensity, L,N) + specular(light_intensity, shine_factor, L, N, H)), 1);
}