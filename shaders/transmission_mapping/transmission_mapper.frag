#version 460 core

layout (location = 0) out vec4 TransmissionMap;
layout (location = 1) out vec4 LightDepthMap;

uniform vec4 TranslucentColour;
uniform float NearPlane;
uniform float FarPlane;

float linearize_depth(float d,float zNear,float zFar)
{
    return zNear * zFar / (zFar + d * (zNear - zFar));
}

void main()
{
    vec4 colour = vec4(TranslucentColour.rgb * TranslucentColour.a, TranslucentColour.a);
    vec4 depth = vec4(linearize_depth(gl_FragDepth, NearPlane, FarPlane),0,0,1);
    
    float condition = float(TranslucentColour.a < 0.999f);
    
    TransmissionMap = (condition * colour) + ((1.0 - condition) * vec4(0,0,0,1));
    LightDepthMap = (condition * depth) + ((1.0 - condition) * vec4(0,0,0,0));
    
//    if (TranslucentColour.a >= 1.0f)
//    {
//        TransmissionMap.rgb = vec3(0.0);
//        TransmissionMap.a = 1.0f;
//        LightDepthMap = vec4(0,0,0,0);
//    }
//    else {
//        TransmissionMap = TranslucentColour;
//        TransmissionMap.rgb *= TransmissionMap.a;
//        LightDepthMap = vec4(linearize_depth(gl_FragDepth, NearPlane, FarPlane),0,0,1);
//    }
}