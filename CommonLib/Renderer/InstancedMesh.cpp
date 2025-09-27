#include "InstancedMesh.h"

#include "RendererConstants.hpp"

InstancedMesh& InstancedMesh::operator=(VertexData&& data)
{
    vertices   = std::move(data.vertices);
    indices    = std::move(data.indices);
    colours    = std::move(data.colours);
    texCoords  = std::move(data.texCoords);
    lightMapUV = std::move(data.lightMapUV);
    normals    = std::move(data.normals);
    tangents   = std::move(data.tangents);
    binormals  = std::move(data.binormals);

    return *this;
}

void InstancedMesh::Build()
{
    glCreateVertexArrays(1, &m_VAO);
    int bindingIDX = 0;
    if (vertices.size() > 0)
    {
        const auto attribIndex = Constants::Renderer::VERTEX_CONSTANTS.AttribIndex.POSITION;
        glCreateBuffers(1, &m_VBO);
        glNamedBufferStorage(m_VBO, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_MAP_READ_BIT);    
        glEnableVertexArrayAttrib(m_VAO, attribIndex);
        glVertexArrayAttribFormat(m_VAO, attribIndex, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(m_VAO, attribIndex, bindingIDX);
        glVertexArrayVertexBuffer(m_VAO, bindingIDX, m_VBO, 0, sizeof(glm::vec3));
        bindingIDX++;
    }
    
    if (colours.size() > 0)
    {
        const auto attribIndex = Constants::Renderer::VERTEX_CONSTANTS.AttribIndex.COLOUR;
        glCreateBuffers(1, &m_CBO);
        glNamedBufferStorage(m_CBO, colours.size() * sizeof(glm::vec4), colours.data(), GL_MAP_READ_BIT);    
        glEnableVertexArrayAttrib(m_VAO, attribIndex);
        glVertexArrayAttribFormat(m_VAO, attribIndex, 4, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(m_VAO, attribIndex, bindingIDX);
        glVertexArrayVertexBuffer(m_VAO, bindingIDX, m_CBO, 0, sizeof(glm::vec4));
        bindingIDX++;
    }

    if (normals.size() > 0)
    {
        const auto attribIndex = Constants::Renderer::VERTEX_CONSTANTS.AttribIndex.NORMAL;
        glCreateBuffers(1, &m_NBO);
        glNamedBufferStorage(m_NBO, normals.size() * sizeof(glm::vec3), normals.data(), GL_MAP_READ_BIT);    
        glEnableVertexArrayAttrib(m_VAO, attribIndex);
        glVertexArrayAttribFormat(m_VAO, attribIndex, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(m_VAO, attribIndex, bindingIDX);
        glVertexArrayVertexBuffer(m_VAO, bindingIDX, m_NBO, 0, sizeof(glm::vec3));
        bindingIDX++;
    }

    if (indices.size() > 0)
    {
        glCreateBuffers(1, &m_IBO);
        glNamedBufferStorage(m_IBO, indices.size() * sizeof(GLuint), indices.data(), GL_MAP_READ_BIT);    
        glVertexArrayElementBuffer(m_VAO, m_IBO);
    }
    glBindVertexArray(m_VAO);

    
}

void InstancedMesh::Bind()
{
    glBindVertexArray(m_VAO);
}

void InstancedMesh::Dispatch()
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, nullptr);
}

void InstancedMesh::Unbind()
{
    glBindVertexArray(0);
}

void InstancedMesh::Delete()
{
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_IBO);
    glDeleteBuffers(1, &m_CBO);
    glDeleteBuffers(1, &m_UV0BO);
    glDeleteBuffers(1, &m_UV1BO);
    glDeleteBuffers(1, &m_NBO);
    glDeleteBuffers(1, &m_BNBO);
    glDeleteBuffers(1, &m_TBO);
    glDeleteVertexArrays(1, &m_VAO);
}
