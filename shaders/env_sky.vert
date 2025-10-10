#version 460 core
layout(location=0) in vec3 VERT_POS;
layout(location=1) in vec4 VERT_COLOUR;
layout(location=2) in vec3 VERT_NORMAL;
layout(location=5) in vec2 VTEX_COORDS;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec3 ViewDir;

void main()
{
    // strip translation so the sky stays "fixed" to camera position
    mat3 rotView = mat3(view);
    ViewDir = rotView * VERT_POS;

    // push sky to far depth (so it always renders behind geometry)
    vec4 pos = projection * view * model * vec4(VERT_POS, 1.0);
    gl_Position = pos.xyww;
}