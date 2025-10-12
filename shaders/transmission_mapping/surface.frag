#version 460 core

in vec4 FRAG_COLOUR;
in vec3 FRAG_NORMAL;
in vec4 PIXEL_POSITION;
in vec2 TexCoords;
in vec4 PIXEL_POSITION_LIGHT_SPACE;

uniform sampler2D ShadowMap;
uniform sampler2D TransmissionMap;
uniform sampler2D LightDepthMap;
uniform float pcf_kernel_width;

uniform float light_intensity;
uniform vec3 LightPos;
uniform vec3 CameraPosition;
uniform vec4 TranslucentColour;

out vec4 fragColour;

const vec3 random_colour_that_is_deprecated = vec3(0.985, 0.6, 0.5);


vec3 lambert_diffuse(in vec3 albedo, in float intensity, in float dotNL)
{
    const vec3 diffuse = max(dotNL, 0.0f) * albedo * intensity;
    return diffuse;
}

vec3 blinn_phong_specular(in float intensity, in float shininess, in float dotNH)
{
    return pow(max(dotNH, 0.0), shininess) * FRAG_COLOUR.rgb * intensity;
}

float ShadowCalculation(in sampler2D shadowMapRef, in vec4 fragPosLightSpace, in float dotNL, in vec3 L)
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
    float bias = max(0.001 * (1.0 - dotNL), 0.0001);

    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    vec2 texelSize = 1.0 / textureSize(shadowMapRef, 0);
    const float pcf_half_size = floor(pcf_kernel_width/2.0);

    // world distance from fragment to light
    float fragDist = length(LightPos - PIXEL_POSITION.xyz);

    const float maxShadowDistance = 2000.0f; // tweak for your scene
    float lightDistance = distance(PIXEL_POSITION.xyz, L);
    float distance_lerp = clamp(fragDist / maxShadowDistance, 0.0, 1.0);

    // dynamic kernel radius: 1 near, pcf_half_size far
    float size = mix(1.0, pcf_half_size, distance_lerp);

    int taps = 0;
    for(float x = -size; x <= size; ++x)
    {
        for(float y = -size; y <= size; ++y)
        {
            float pcfDepth = texture(shadowMapRef, projCoords.xy + vec2(x, y) * texelSize).r;
            shadowing += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
            taps++;
        }
    }
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    //    shadowing /= (pcf_kernel_width * pcf_kernel_width); // <=> 1/(kernel_width^2)
    shadowing /= float(taps); // <=> 1/(kernel_width^2)
    if (projCoords.z > 1.0f)
    {
        return 0.0f;
    }

    return shadowing;
}

vec3 TransmissiveCalculation(in sampler2D transmissionMapRef, in sampler2D shadowMapRef , in vec4 fragPosLightSpace, in float dotNL, in vec3 L)
{
    vec3 result = vec3(0.0f);

    // perform perspective divide - Helps to support ortho and perspective projections.
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;  // transform to [0,1] range
    
    if (projCoords.z > 1.0f)
    {
        return result;
    }
    
    vec4 colour = texture(transmissionMapRef, (projCoords.xy - 0.5) + 0.5);
    
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMapRef, projCoords.xy).r;

    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // calculate bias (based on depth map resolution and slope)
    float bias = max(0.001 * (1.0 - dotNL), 0.0001);

    result += currentDepth - bias > closestDepth ? colour.rgb : vec3(1.0);
    
    return result;
}

vec3 saturate(in vec3 value)
{
    return clamp(value, vec3(0.0), vec3(1.0));
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

    vec3 ambient_colour = TranslucentColour.rgb * vec3(0.3);

    float depthShadow = ShadowCalculation(ShadowMap, PIXEL_POSITION_LIGHT_SPACE, dotNL, L);
    float opaqueShadow = ShadowCalculation(LightDepthMap, PIXEL_POSITION_LIGHT_SPACE, dotNL, L);
    vec3 light = TransmissiveCalculation(TransmissionMap, ShadowMap, PIXEL_POSITION_LIGHT_SPACE, dotNL, L);
    vec3 projectedLight = saturate(light * (vec3(1.0) - saturate(vec3(depthShadow * opaqueShadow))));
    
    vec3 diffuse = lambert_diffuse(TranslucentColour.rgb + TranslucentColour.a, light_intensity, dotNL);
    vec3 specular = blinn_phong_specular(light_intensity, 68.0f, dotNH);
    
    fragColour.rgb = (ambient_colour + (projectedLight * light_intensity)) * ( diffuse + specular );
    fragColour.a = TranslucentColour.a;
}