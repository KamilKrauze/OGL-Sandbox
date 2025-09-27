#ifndef INST_MESH_H
#define INST_MESH_H

#include "IRenderable.h"

#include <vector>

#include <glm/glm.hpp>

#include "VertexData.h"

/**
 * Mesh objects that are instantiated.
 */
class InstancedMesh : public IRenderable
{
public:
    InstancedMesh() = default;
    ~InstancedMesh() override = default;
    InstancedMesh(InstancedMesh&& other) noexcept = default;

    /**
     * Helps move vertex data generated/loaded into the mesh object with ease.
     * @param data Vertex data
     * @return Mesh instance self ref.
     */
    InstancedMesh& operator=(VertexData&& data);
    
// Graphics API calls
public:
    void Build() override;
    void Bind() override;
    void Dispatch() override;
    void Unbind() override;
    void Delete() override;

// Raw vertex data 
public:
    GLenum bufferUsage = GL_MAP_READ_BIT;
    std::vector<glm::vec3>  vertices;
    std::vector<GLuint>     indices;
    std::vector<glm::vec4>  colours;
    std::vector<glm::vec2>  texCoords;
    std::vector<glm::vec2>  lightMapUV;
    std::vector<glm::vec3>  normals;
    std::vector<glm::vec3>  tangents;
    std::vector<glm::vec3>  binormals;
};

#endif