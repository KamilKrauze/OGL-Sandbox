#include "GBuffer.h"
#include "Logger.hpp"
#include "Renderer/RendererStatics.hpp"

void GBuffer::Create()
{
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    auto& [width, height] = RendererStatics::WindowDimensions;
    BindBuffers(width, height);
    
    // Check completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        LOG_ERROR("Framebuffer is not complete!");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GBuffer::BindBuffers(int width, int height)
{
    // Albedo + Emissive (RGBA8)
    glGenTextures(1, &gAlbedoEmissive);
    glBindTexture(GL_TEXTURE_2D, gAlbedoEmissive);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gAlbedoEmissive, 0);
    
    // Position (RGB16F for precision)
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gPosition, 0);

    // Normal (RGB16F)
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gNormal, 0);

    // SRMAO (RGB16F)
    glGenTextures(1, &gSRMAO);
    glBindTexture(GL_TEXTURE_2D, gSRMAO);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gSRMAO, 0);
    
    // Depth Renderbuffer
    glGenTextures(1, &gDepth);
    glBindTexture(GL_TEXTURE_2D, gDepth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = {1.0, 1.0, 1.0, 1.0};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    // Make sure the texture is sampled as a float texture (not shadow compare mode) if you sample with sampler2D
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

    // Attach as *depth* attachment, not color!
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gDepth, 0);

    // Tell OpenGL which color attachments to draw into
    glDrawBuffers(attachments.size(), attachments.data());
}

void GBuffer::RecreateBuffers(int width, int height)
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    DeleteInternal();
    BindBuffers(width, height);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        LOG_ERROR("Framebuffer not complete after resize!");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GBuffer::RecreateBuffersOnWindowResize(WindowResizePayload& payload)
{
    RecreateBuffers(payload.width, payload.height);
}

void GBuffer::ReadBuffer(GLuint& program)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gAlbedoEmissive);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, gSRMAO);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, gDepth);
    glUniform1i(glGetUniformLocation(program, "gAlbedoEmissive"), 0);
    glUniform1i(glGetUniformLocation(program, "gPosition"), 1);
    glUniform1i(glGetUniformLocation(program, "gNormal"), 2);
    glUniform1i(glGetUniformLocation(program, "gSRMAO"), 3);
    glUniform1i(glGetUniformLocation(program, "depth"), 4);
}

void GBuffer::Delete()
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    DeleteInternal();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(0, &framebuffer);
}

void GBuffer::DeleteInternal()
{
    // Detach any existing color attachments from the FBO first
    // (This prevents the FBO from holding references to the old textures which delays their deletion)
    if (gAlbedoEmissive)   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
    if (gPosition)     glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, 0, 0);
    if (gNormal)       glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, 0, 0);
    if (gSRMAO)       glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, 0, 0);
    if (gDepth)        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,     GL_TEXTURE_2D, 0, 0);
    
    // delete old attachments
    glDeleteTextures(1, &gAlbedoEmissive);
    glDeleteTextures(1, &gPosition);
    glDeleteTextures(1, &gNormal);
    glDeleteTextures(1, &gSRMAO);
    glDeleteTextures(1, &gDepth);

    gAlbedoEmissive = 0;
    gPosition = 0;
    gNormal = 0;
    gSRMAO = 0;
    gDepth = 0;
}
