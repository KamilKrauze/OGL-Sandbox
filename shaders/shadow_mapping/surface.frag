#version 460 core

in vec4 FRAG_COLOUR;
in vec3 FRAG_NORMAL;
in vec4 PIXEL_POSITION;
in vec2 TexCoords;
in vec4 PIXEL_POSITION_LIGHT_SPACE;

uniform sampler2D shadowMap;

uniform float light_intensity;
uniform vec3 LightPos;
uniform vec3 CameraPosition;

out vec4 fragColour;

const vec3 random_colour_that_is_deprecated = vec3(0.985, 0.6, 0.5);
vec3 ambient_colour = vec3(0.1);

vec3 lambert_diffuse(in vec3 albedo, in float intensity, in float dotNL)
{
    const vec3 diffuse = max(dotNL, 0.0f) * albedo * intensity;
    return diffuse;
}

vec3 blinn_phong_specular(in float intensity, in float shininess, in float dotNH)
{
    return pow(max(dotNH, 0.0), shininess) * FRAG_COLOUR.rgb * intensity;
}

float ShadowCalculation(in sampler2D shadowMapRef, in vec4 fragPosLightSpace, in float dotNL)
{
    float shadowing = 0.0f;
    
    // perform perspective divide - Helps to support ortho and perspective projections.
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;  // transform to [0,1] range

    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMapRef, projCoords.xy).r;

    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // calculate bias (based on depth map resolution and slope)
    float bias = max(0.01 * (1.0 - dotNL), 0.005);

    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    vec2 texelSize = 1.0 / textureSize(shadowMapRef, 0);
    const int size = 1;
    for(int x = -size; x <= size; ++x)
    {
        for(int y = -size; y <= size; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadowing += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
        }
    }
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    shadowing /= 9; // <=> 1/9
    if (projCoords.z > 1.0f)
    {
        return 0.0f;
    }
    
    return shadowing;
}

void main()
{
    const vec3 N = normalize(FRAG_NORMAL);
    const vec3 L = normalize(LightPos - PIXEL_POSITION.xyz);
    const vec3 V = normalize(CameraPosition - PIXEL_POSITION.xyz);
    const vec3 R = reflect(-L, N);
    const vec3 H = (normalize(L + V));
    const float dotNL = dot(N,L);
    const float dotNH = dot(N,H);

    float lightDistance = distance(PIXEL_POSITION.xyz, L);
    
    float shadow = ShadowCalculation(shadowMap, PIXEL_POSITION_LIGHT_SPACE, dotNL);
    
    fragColour.rgb = (ambient_colour + (1-shadow)) * (lambert_diffuse(FRAG_COLOUR.rgb, light_intensity, dotNL) +
    blinn_phong_specular(light_intensity, 128.0f, dotNH));
//    fragColour.rgb = texture(shadowMap, PIXEL_POSITION_LIGHT_SPACE.xy).rrr;
//    fragColour.rgb = vec3(texture(shadowMap, TexCoords).r);
    
    fragColour.a = 1.0f;
}