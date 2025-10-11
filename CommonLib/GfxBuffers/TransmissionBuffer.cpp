#include "TransmissionBuffer.h"
#include "Logger.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Renderer/RendererStatics.hpp"

static constexpr uint32_t MAP_RESOLUTION = 1024;

TransmissionBuffer::~TransmissionBuffer()
{
    Destroy();
}

void TransmissionBuffer::Create()
{
    glGenFramebuffers(1, &lightFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, lightFBO);

    // Depth texture (for shadow test)
    glGenTextures(1, &shadowMap);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24,
                 MAP_RESOLUTION, MAP_RESOLUTION, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = {1.0, 1.0, 1.0, 1.0};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);


    // Transmission texture (color output)
    glGenTextures(1, &transmissionMap);
    glBindTexture(GL_TEXTURE_2D, transmissionMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
                 MAP_RESOLUTION, MAP_RESOLUTION, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, transmissionMap, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);

    // Only one color buffer
    GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        LOG_ERROR("Light FBO incomplete!");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void TransmissionBuffer::Bind()
{
    glViewport(0, 0, MAP_RESOLUTION, MAP_RESOLUTION);
    glBindFramebuffer(GL_FRAMEBUFFER, lightFBO);

    GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);
}

void TransmissionBuffer::BindData(uint32_t& shader, const int32_t& shadowMapBinding, const int32_t& transmissionMapBinding)
{
    if (shadowMapBinding == transmissionMapBinding)
    {
        LOG_ERROR("[TransmitBuffer/{%ul}]Bad binding values... shadowMapBinding (%d) != transmissionMapBinding (%d)", lightFBO, shadowMapBinding, transmissionMapBinding);
        return;
    }
    glUniformMatrix4fv(glGetUniformLocation(shader, "LightSpaceMatrix"), 1, GL_FALSE, &lightSpaceMatrix[0][0]);
    
    glActiveTexture(GL_TEXTURE0 + shadowMapBinding); // Index from 0.
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    glUniform1i(glGetUniformLocation(shader, "ShadowMap"), shadowMapBinding);

    glActiveTexture(GL_TEXTURE0 + transmissionMapBinding); // Index from 0.
    glBindTexture(GL_TEXTURE_2D, transmissionMap);
    glUniform1i(glGetUniformLocation(shader, "TransmissionMap"), transmissionMapBinding);
}

void TransmissionBuffer::Update(const glm::vec3& light_pos)
{
    lightProjection = glm::ortho(-projection_fov, projection_fov, -projection_fov, projection_fov, near_plane, far_plane);
    lightView = glm::lookAt(light_pos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
    lightSpaceMatrix = lightProjection * lightView;
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "LightSpaceMatrix"), 1, GL_FALSE, &lightSpaceMatrix[0][0]);   

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(2.0f, 4.0f);
}

void TransmissionBuffer::Unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    int width, height;
    glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height); // safer and live
    glViewport(0, 0, width, height);
}

void TransmissionBuffer::Destroy()
{
    if (!isDestroyed)
    {
        glDeleteTextures(1, &shadowMap);
        glDeleteTextures(1, &transmissionMap);
        glDeleteFramebuffers(1, &lightFBO);
        glDeleteProgram(shaderProgram);
        isDestroyed = true;
    }
}
