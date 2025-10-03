#version 460 core

layout (location = 0) out vec4 gAlbedoEmissive;
layout (location = 1) out vec3 gPosition;
layout (location = 2) out vec3 gNormal;
layout (location = 3) out vec4 gSRMAO;

layout(binding=10) uniform sampler2D diffuse;
layout(binding=11) uniform sampler2D ARM;


in vec3 FragPos;    // World-space position
in vec3 Normal;     // World-space normal
in vec2 TexCoords;  // Texture coordinates
in vec4 VertexColour; // Vertex colours.

void main()
{
    vec4 armPixel = texture(ARM, TexCoords); 
    gAlbedoEmissive.rgb = texture(diffuse, TexCoords).rgb; // Albedo ( vertex colour for now :-) )
    gAlbedoEmissive.a = 0.0f; // Emisiveness
    
    gPosition.rgb = FragPos; // Pixel position
    gNormal = normalize(Normal); // Normal
    gSRMAO.r = 1.0f; // Specular
    gSRMAO.g = armPixel.g; // Roughness
    gSRMAO.b = armPixel.b; // Metallic
    gSRMAO.a = armPixel.r; // AO
}