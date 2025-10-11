#ifndef TRANSLUCENCY_BUFFER_H
#define TRANSLUCENCY_BUFFER_H

#include <cstdint>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class TransmissionBuffer
{
public:
    TransmissionBuffer() = default;
    ~TransmissionBuffer();

public:
    /**
     * Creates the necessary buffers.
     */
    void Create();

    /**
     * Binds the frame-buffer object
     */
    void Bind();

    /**
     * Binds the buffer textures to some shader
     * @param shader Specified shader
     * @param binding Shader binding location
     */
    void BindData(uint32_t& shader, const int32_t& shadowMapBinding, const int32_t& transmissionMapBinding);
    
    /**
     * Updates internal values
     * @remarks Light projection matrices
     */
    void Update(const glm::vec3& light_pos);
    void Unbind();
    void Destroy();

public:
    float near_plane = 0.1f;
    float far_plane = 20.0f;
    float pcf_kernel_width = 5;
    float projection_fov = 5.0f;
    glm::mat4 lightSpaceMatrix;

    uint32_t shaderProgram;
    uint32_t shadowMap;
    uint32_t transmissionMap;

private:
    glm::mat4 lightProjection;
    glm::mat4 lightView;
    
    // --- Buffer Object References
private:
    uint32_t lightFBO;

private:
    bool isDestroyed = false;
};

#endif //!TRANSLUCENCY_BUFFER_H