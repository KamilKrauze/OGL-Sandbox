#version 460 core

in vec2 TexCoords;
in vec3 light_pos;
out vec4 fragColour;

// G-Buffer textures
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D depth;

const float light_intensity = 0.5f;

vec3 PIXEL_POSITION;
vec3 PIXEL_NORMALS;
vec3 PIXEL_SPECULAR;
float PIXEL_DEPTH;

vec3 norm_normals;
vec3 norm_lightpos;

vec3 V;
vec3 R;
vec3 H;

vec4 diffuse_colour()
{
    vec4 diffuse = max(dot(norm_normals, norm_lightpos), 0.0f) * vec4(vec3(light_intensity),1) * vec4(PIXEL_NORMALS,1);
    return diffuse;
}

vec4 specular(float shininess)
{
    return pow(max(dot(norm_normals,H), 0.0), shininess) * light_intensity * vec4(PIXEL_SPECULAR, 1) * vec4(PIXEL_NORMALS,1);
}

float linearize_depth(float depth, float nearPlane, float farPlane)
{
    float z = depth * 2.0 - 1.0;
    float linear_depth = (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));

    return linear_depth;
}

void main()
{
    PIXEL_POSITION = texture(gPosition, TexCoords).rgb;
    PIXEL_NORMALS = texture(gNormal, TexCoords).rgb;
    PIXEL_SPECULAR = texture(gAlbedoSpec, TexCoords).rgb;
    PIXEL_DEPTH = texture(depth, TexCoords).r;
    float linear_depth = linearize_depth(PIXEL_DEPTH, 0.001f, 1000.0f) / 1000;
    float inv_depth = (1-linear_depth);
    
    norm_normals = normalize(PIXEL_NORMALS.rgb);
    norm_lightpos = normalize(light_pos);

    V = normalize(-texture(gPosition, TexCoords).rgb);
    R = reflect(-norm_lightpos, norm_normals);
    H = (light_pos + V) / (normalize(light_pos + V)); 
    
    fragColour = vec4((diffuse_colour().rgb + specular(3).rgb), 1);
}