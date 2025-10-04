#version 460 core

in vec2 TexCoords;
in vec3 light_pos;
out vec4 fragColour;

// G-Buffer textures
uniform sampler2D gAlbedoEmissive;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gSRMAO;
uniform sampler2D depth;

uniform sampler2D EnvSphereTexture;

uniform vec3 CameraPosition;
uniform float shine_factor;
uniform float metallic;
uniform float light_intensity;
uniform float fresnel_coeff;
uniform float fresnel_factor;

vec3 ALBEDO;
vec4 SRMAO;
vec3 PIXEL_POSITION;
vec3 PIXEL_NORMALS;
float PIXEL_DEPTH;

vec3 lambert_diffuse(vec3 albedo, float intensity, vec3 L, vec3 N)
{
    vec3 diffuse = max(dot(N, L), 0.0f) * albedo * intensity;
    return diffuse;
}

vec3 blinn_phong_specular(float intensity, float shininess, vec3 L, vec3 N, vec3 H)
{
    return pow(max(dot(N,H), 0.0), shininess) * vec3(0.985, 0.6, 0.5) * intensity;
}

vec3 schlick_fresnel(vec3 R0, float cos_i)
{
    return R0 + ((1-R0)*pow(1-cos_i, 5.0));
}
const float PI = 3.14159265359;

vec3 beckmann_distribution(vec3 N, vec3 H, float roughness)
{
    vec3 result = vec3(0);
    const float alpha = max(dot(N,H),0.0);
    if (alpha <= 0.0)
    {
        return result;
    }
    const float m = max(0.01, roughness * roughness);
    const float m2 = m*m;
    
    const float cos2 = alpha * alpha;
    const float tan2 = (1.0-cos2) / cos2;
    const float exponent = -tan2 / m2;
    const float numerator = exp(exponent);
    const float denominator = PI * m2 * cos2 * cos2;
    
    result = vec3(numerator / denominator);
    
    return result;
}

vec3 geometric_attenuation(vec3 N, vec3 V, vec3 L, vec3 H)
{
    vec3 result = vec3(0);
    
    const float dotHN = max(dot(H,N),0.001);
    const float dotLN = max(dot(L,N),0.001);
    const float dotVN = max(dot(V,N),0.001);
    const float dotVH = max(dot(V,H),0.001);
    
    const float g1 = (2 * dotHN * dotVN) / dotVH;
    const float g2 = (2 * dotHN * dotLN) / dotVH;
    
    result = vec3(min(1,min(g1,g2)));
    
    return result;
}

vec3 cook_torrance_brdf(vec3 N, vec3 V, vec3 L, vec3 H, vec3 R0, float roughness)
{
    vec3 result = vec3(0);
    
    const float dotNL = max(dot(N,L),0.01);
    const float dotVN = max(dot(V,N),0.01);
    
    float cosTheta = max(dot(N,V), 0);
    const vec3 F = schlick_fresnel(R0, cosTheta);
    const vec3 D = beckmann_distribution(N,H,roughness);
    const vec3 G = geometric_attenuation(N,V,L,H);
    
    const vec3 numerator = D * F * G;
    const vec3 denominator = 4.0 * vec3(dotVN) * vec3(dotNL);
    result = (numerator/max(denominator,0.001));    
    
    return result;
}

vec3 burley_diffuse(vec3 N, vec3 V, vec3 L, vec3 H, vec3 albedo, float roughness)
{
    const float dotLN = max(dot(L,N),0.001);
    const float dotVN = max(dot(V,N),0.001);
    const float dotHL = max(dot(H,L),0.001);
    
    const float F0 = 0.5 + (2.0 * roughness * pow(dotHL,2.0));
    const float F90 = 1.0;
    
    vec3 diffuse = mix(F0, F90, dotLN) * mix(F0, F90, dotVN) * albedo;

    return diffuse;
}

// In your lighting fragment shader (GLSL)
float LinearizeDepth(float depth)
{
    const float near = 0.001;
    const float far = 1000000.0;

    // if depth is the NDC / window depth (0..1), convert to NDC z in [-1,1]
    float z_ndc = depth * 2.0 - 1.0;
    // then recover eye-space z (negative along -Z if using standard OpenGL)
    // alternative robust formula:
    float linear = (2.0 * near * far) / (far + near - z_ndc * (far - near));
    return linear; // linear in view-space distance
}

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

vec2 sampleEquirect(vec3 dir) {
    float phi = atan(dir.z, dir.x); // longitude
    float theta = asin(dir.y);      // latitude
    vec2 uv = vec2(phi / (2.0 * 3.1415926) + 0.5, theta / 3.1415926 + 0.5);
    return uv;
}

float rand(vec2 n){
    return fract(sin(dot(n, vec2(127.1, 311.7))) * 43758.5453123);
}

vec3 hemisphereSample(vec3 normal, vec2 xi) {
    // xi = random numbers in [0,1]
    float phi = 2.0 * 3.1415926 * xi.x;
    float cosTheta = sqrt(1.0 - xi.y);  // cosine-weighted
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    // spherical -> cartesian
    vec3 tangentSample = vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);

    // create tangent space basis
    vec3 up = abs(normal.z) < 0.999 ? vec3(0.0,0.0,1.0) : vec3(1.0,0.0,0.0);
    vec3 tangentX = normalize(cross(up, normal));
    vec3 tangentY = cross(normal, tangentX);

    // transform sample to world space
    return tangentX * tangentSample.x + tangentY * tangentSample.y + normal * tangentSample.z;
}

void main()
{   
    ALBEDO = texture(gAlbedoEmissive, TexCoords).rgb;
    SRMAO = texture(gSRMAO, TexCoords);
    PIXEL_POSITION = texture(gPosition, TexCoords).rgb;
    PIXEL_NORMALS = texture(gNormal, TexCoords).rgb;
    PIXEL_DEPTH = texture(depth, TexCoords).r;

    if (PIXEL_DEPTH >= 1.0f)
    {
        discard; return;
    }
    
    vec3 N = normalize(PIXEL_NORMALS);
    vec3 L = normalize(light_pos - PIXEL_POSITION);

    vec3 V = normalize(CameraPosition - PIXEL_POSITION);
    vec3 R = reflect(-PIXEL_POSITION, N);
    vec3 H = (normalize(L + V));

    const vec2 ENV_UV = sampleEquirect(L);
    vec3 irradiance = vec3(0.0);
    const int SAMPLE_COUNT = 64;
    for(int i = 0; i < SAMPLE_COUNT; ++i) {
        vec2 xi = vec2(
            rand(gl_FragCoord.xy + float(i)*vec2(12.9898,78.233)),
            rand(gl_FragCoord.xy + float(i)*vec2(39.346, 98.123))
        );
        vec3 sampleDir = hemisphereSample(N, xi);
        vec2 uv = sampleEquirect(sampleDir);
        vec3 sampleColour =  texture(EnvSphereTexture, uv).rgb;
        irradiance += sampleColour;
    }
    irradiance /= float(SAMPLE_COUNT);
    irradiance = toneMapACES(irradiance, 1.0f);
    
    float ROUGHNESS = max(0.1, (SRMAO.g) * shine_factor);

    float cosTheta = max(dot(N,V), 0);
//    fragColour = vec4((lambert_diffuse(ALBEDO, light_intensity, L,N) + blinn_phong_specular(light_intensity, ROUGHNESS, L, N, H)), 1);
//    fragColour = vec4(ALBEDO + min(schlick_fresnel(vec3(fresnel_coeff), cosTheta * fresnel_factor),0.0), 1);
//    fragColour = vec4(vec3(RM(lambert_diffuse(vec3(0.985, 0.2, 0.15), light_intensity, L,N))),1);
//    fragColour= vec4(ALBEDO + beckmann_distribution(N,H,light_intensity) ,1);
//    fragColour= vec4(ALBEDO + geometric_attenuation(N,V,L,H) ,1);
    
    
//    fragColour = vec4(lambert_diffuse(ALBEDO * SRMAO.a, light_intensity, L,N) + cook_torrance_brdf(N,V,L,H, vec3(fresnel_coeff), ROUGHNESS) * light_intensity * max(dot(N,L), 0.0), 1);
    
    vec3 diffuse = burley_diffuse(N, L, V, H, ALBEDO * SRMAO.a, ROUGHNESS);
    vec3 specular = cook_torrance_brdf(N, V, L, H, vec3(fresnel_coeff), ROUGHNESS);
    fragColour.rgb = (diffuse * vec3(1-(SRMAO.b - metallic))) + (specular * mix(vec3(1.0), ALBEDO, (SRMAO.b - metallic))) * light_intensity + (ALBEDO * irradiance);
//    fragColour.rgb = toneMapACES(fragColour.rgb, 1);
    fragColour.a = 1;
}