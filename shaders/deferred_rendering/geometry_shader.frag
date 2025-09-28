#version 460 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec3 FragPos;    // world-space position
in vec3 Normal;     // world-space normal
in vec2 TexCoords;  // texture coordinates
in vec4 VertexColour; // Vertex colours.

void main()
{
    gPosition    = FragPos;
    gNormal      = normalize(Normal);
    gAlbedoSpec = vec4(0.5, 0.5, 0.5, 1);
}