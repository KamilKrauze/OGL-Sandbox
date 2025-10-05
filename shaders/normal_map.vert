#version 460 core
layout(location=0) in vec3 VERT_POS;
layout(location=1) in vec4 VERT_COLOUR;
layout(location=2) in vec3 VERT_NORMAL;
layout(location=3) in vec3 VERT_TANGENT;
layout(location=4) in vec3 VERT_BITANGENT;
layout(location=5) in vec2 VTEX_COORDS;

out vec2 TEX_COORDS;
out vec3 PIXEL_POSITION;
out vec3 PIXEL_NORMALS;
out vec3 PIXEL_TANGENTS;
out vec3 PIXEL_BITANGENTS;

out mat3 TBNMatrix;
out vec3 TANGENT_PIXEL_POS;
out vec3 TANGENT_LIGHT_POS;
out vec3 TANGENT_VIEW_POS;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normal_matrix;
uniform vec3 CameraPos;
uniform vec3 LightPos;

void main()
{
    const vec4 POSITION_H = vec4(VERT_POS, 1);
    TEX_COORDS = VTEX_COORDS;
    
    vec3 T = normalize(normal_matrix * VERT_TANGENT);
    vec3 N = normalize(normal_matrix * VERT_NORMAL);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    TBNMatrix = transpose(mat3(T, B, N));

    PIXEL_POSITION = vec3((model * POSITION_H));
    PIXEL_NORMALS = N;
    PIXEL_TANGENTS = T;
    PIXEL_BITANGENTS = B;
    
    TANGENT_LIGHT_POS = TBNMatrix * LightPos;
    TANGENT_VIEW_POS = TBNMatrix * CameraPos;
    TANGENT_PIXEL_POS = TBNMatrix * PIXEL_POSITION;
    
    gl_Position = projection * view * model * POSITION_H;
}