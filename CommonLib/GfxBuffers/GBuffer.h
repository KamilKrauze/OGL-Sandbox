#ifndef GBUFFER_HPP
#define GBUFFER_HPP
#include <vector>
#include <glad/glad.h>

#include "DelegateHandlers/GeneralSystemEvents.h"

class GBuffer
{
public:
    GBuffer() = default;
    ~GBuffer() = default;
public:
    void Create();
    void BindBuffers(int width, int height);
    void RecreateBuffers(int width, int height);
    void RecreateBuffersOnWindowResize(WindowResizePayload& payload);
    void ReadBuffer(GLuint& program);
    void Delete();
private:
    void DeleteInternal();
public:
    GLuint shader = 0;
    unsigned int framebuffer = 0;
    unsigned int gAlbedoEmissive = 0; // Albedo, Emissiveness.
    unsigned int gPosition = 0;
    unsigned int gNormal = 0;
    unsigned int gSRMAO = 0; // Specular, Roughness, Metallic, Ambient Occlusion.
    unsigned int gDepth = 0;
    std::vector<GLuint> attachments = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
};

#endif//! GBUFFER_HPP
