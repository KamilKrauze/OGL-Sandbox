#version 460 core

in vec2 TEX_COORDS;
in vec3 PIXEL_POSITION;
in vec3 PIXEL_NORMALS;
in vec3 PIXEL_TANGENTS;
in vec3 PIXEL_BITANGENTS;

in mat3 TBNMatrix;
in vec3 TANGENT_PIXEL_POS;
in vec3 TANGENT_LIGHT_POS;
in vec3 TANGENT_VIEW_POS;

out vec4 FragColour;

uniform sampler2D AlbedoMap;
uniform sampler2D ARM_Map;
uniform sampler2D NormalMap;

uniform float light_intensity;

//void main()
//{
//    vec3 normal = texture(NormalMap, TEX_COORDS).rgb;
//    normal = normalize(normal * 2.0 - 1.0f); // Perturb normals;
//    
//    vec3 lightpos = vec3(0, 0.5f, -1.0f);
//    
//    vec3 lightDir = TBNMatrix * normalize(lightPos - PIXEL_POSITION);
//    vec3 viewDir  = TBNMatrix * normalize(viewPos - PIXEL_POSITION);
//    
//    FragColour.rgb = normal;
//    FragColour.a = 1.0f;
//}

void main()
{
    const vec3 ARM = texture(ARM_Map, TEX_COORDS).rgb;
    const float AO = ARM.r;
    const float ROUGHNESS = ARM.g;
    const float METAL = ARM.b;
    // obtain normal from normal map in range [0,1]
    vec3 normal = texture(NormalMap, TEX_COORDS).rgb;
    // transform normal vector to range [-1,1]
    normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space

    // get diffuse color
    vec3 color = texture(AlbedoMap, TEX_COORDS).rgb * ARM.r;
    // ambient
    vec3 ambient = 0.3f * color;
    // diffuse
    vec3 lightDir = normalize(TANGENT_LIGHT_POS - TANGENT_PIXEL_POS);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color * light_intensity;
    // specular
    vec3 viewDir = normalize(TANGENT_VIEW_POS - TANGENT_PIXEL_POS);
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 8);

    vec3 specular = vec3(spec * ROUGHNESS * METAL);
    FragColour = vec4(ambient + diffuse + specular, 1.0);
}