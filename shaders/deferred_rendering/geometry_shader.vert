#version 460 core
layout(location=0) in vec3 VERT_POS;
layout(location=1) in vec4 VERT_COLOUR;
layout(location=2) in vec3 VERT_NORMAL;
layout(location=3) in vec3 VERT_TANGENT;
layout(location=4) in vec3 VERT_BITANGENT;
layout(location=5) in vec2 VTEX_COORDS;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat3 normal_matrix;

out vec3 FragPos;    // world-space Position
out vec3 Normal;     // world-space Normal
out vec3 Tangent;     // world-space Tangent
out vec3 BiTangent;     // world-space BiTangent
out vec2 TexCoords;  // texture coordinates
out vec4 VertexColour; // Vertex colours.

void main()
{
    const vec4 worldPos = model * vec4(VERT_POS, 1);

    FragPos = worldPos.xyz;
    VertexColour = VERT_COLOUR;
    TexCoords = VTEX_COORDS;

    Normal = normalize(normal_matrix * VERT_NORMAL);
    Tangent = normalize(normal_matrix * VERT_TANGENT);
    Tangent = normalize(Tangent - dot(Tangent, Normal) * Normal);
    BiTangent = cross(Normal,Tangent);
    
    gl_Position = projection * view * worldPos;
}