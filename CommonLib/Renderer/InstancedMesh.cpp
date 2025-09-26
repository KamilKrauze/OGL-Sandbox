#include "InstancedMesh.h"

#include "RendererConstants.hpp"

void InstancedMesh::Build()
{
    glCreateVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);
    // Vertex position
    if (vertices.size() > 0)
    {
        glCreateBuffers(1, &m_VBO);
        glNamedBufferStorage(m_VBO, vertices.size() * sizeof(glm::vec3), vertices.data(), drawUsage);

        glVertexArrayVertexBuffer(m_VAO, 0, m_VBO, 0, sizeof(glm::vec3));

        glEnableVertexArrayAttrib(m_VAO, Constants::Renderer::VERTEX_CONSTANTS.AttribIndex.POSITION);
        glVertexArrayAttribFormat(m_VAO, Constants::Renderer::VERTEX_CONSTANTS.AttribIndex.POSITION,
            3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(m_VAO, Constants::Renderer::VERTEX_CONSTANTS.AttribIndex.POSITION, 0);
    }
    if (colours.size() > 0)
    {
        glCreateBuffers(1, &m_CBO);
        glNamedBufferStorage(m_CBO, colours.size() * sizeof(glm::vec4), colours.data(), drawUsage);

        // bind colour buffer to binding index 1
        glVertexArrayVertexBuffer(m_VAO, 1, m_CBO, 0, sizeof(glm::vec4));

        glEnableVertexArrayAttrib(m_VAO, Constants::Renderer::VERTEX_CONSTANTS.AttribIndex.COLOUR);
        glVertexArrayAttribFormat(m_VAO, Constants::Renderer::VERTEX_CONSTANTS.AttribIndex.COLOUR,
            4, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(m_VAO, Constants::Renderer::VERTEX_CONSTANTS.AttribIndex.COLOUR, 1);
    }
    if (indices.size() > 0)
    {
        glCreateBuffers(1, &m_IBO);
        glNamedBufferData(m_IBO, indices.size() * sizeof(GLuint), &indices[0], drawUsage);
        glVertexArrayElementBuffer(m_VAO, m_IBO);
    }
}

void InstancedMesh::Bind()
{
    glBindVertexArray(m_VAO);
}

void InstancedMesh::Dispatch()
{
    glBindVertexArray(m_VAO);
    glVertexArrayElementBuffer(m_VAO, m_IBO);
    glDrawElementsInstanced(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, nullptr, 1);
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
