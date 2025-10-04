#version 460 core

in vec3 ViewDir;
out vec4 FragColor;

uniform sampler2D EnvSphereTexture;

vec3 RRTAndODTFit(vec3 v) {
    vec3 a = v * (v + 0.0245786) - 0.000090537;
    vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
    return a / b;
}

vec3 toneMapACES(vec3 color, float exposure) {
    color *= exposure; // <-- exposure control
    color = RRTAndODTFit(color);
    return pow(color, vec3(1.0/2.2)); // gamma to sRGB
}

void main() {
    vec3 dir = normalize(ViewDir);
    
    // Convert direction to spherical coordinates
    float phi   = atan(dir.z, dir.x);      // [-PI, PI]
    float theta = acos(clamp(dir.y, -1.0, 1.0)); // [0, PI]

    // Normalize to [0,1] texture coordinates
    float u = (phi + 3.14159265359) / (2.0 * 3.14159265359);
    float v = theta / 3.14159265359;

    FragColor.rgb = toneMapACES(texture(EnvSphereTexture, vec2(u+0.35f, 1.0-v)).rgb, 1.0f);
    FragColor.a = 1.0f;
}