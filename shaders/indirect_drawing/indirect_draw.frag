#version 460 core

flat in uint objectID;
in vec3 FRAG_NORMAL;

out vec4 FragColour;

// Simple integer hash
uint hash(uint x)
{
    x += 0x9e3779b9u;
    x ^= x >> 16;
    x *= 0x85ebca6bu;
    x ^= x >> 13;
    x *= 0xc2b2ae35u;
    x ^= x >> 16;
    return x;
}

float random(uint seed)
{
    return float(hash(seed)) / float(0xffffffffu);
}


void main()
{
    vec3 color = vec3(
    random(objectID),
    random(objectID + 12345u),
    random(objectID + 67890u)
    );
    
    FragColour = vec4(color * FRAG_NORMAL, 1.0);
}