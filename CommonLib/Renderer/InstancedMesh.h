#pragma once

#include "IRenderable.h"

#include <vector>

#include <glm/glm.hpp>

#include "GfxBuffers/IndexBufferObject.hpp"

class InstancedMesh : IRenderable
{
public:
    InstancedMesh() = default;
    ~InstancedMesh() override = default;
    InstancedMesh(InstancedMesh&& other) noexcept = default;

public:
    

    
// Graphics API calls
public:
    void Build() override;
    void Bind() override;
    void Dispatch() override;
    void Unbind() override;
    void Delete() override;

public:
    GLenum drawUsage = GL_STATIC_DRAW;
    std::vector<glm::vec3>  vertices;
    std::vector<GLuint>     indices;
    std::vector<glm::vec4>  colours;
    std::vector<glm::vec2>  texCoords;
    std::vector<glm::vec2>  lightMapUV;
    std::vector<glm::vec3>  normals;
    std::vector<glm::vec3>  tangents;
    std::vector<glm::vec3>  binormals;
};
