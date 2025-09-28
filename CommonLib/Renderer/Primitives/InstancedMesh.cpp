#include "Renderer/Primitives/InstancedMesh.h"

#include "Logger.hpp"
#include "Renderer/Primitives/MeshPrimitiveUtils.h"
#include "Renderer/RendererConstants.hpp"

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
    if (vertices.size() > 0)
    {
        const auto attribIndex = Constants::Renderer::VERTEX_CONSTANTS.AttribIndex.POSITION;
        MeshPrimitiveUtils::CreateBufferObject(m_VBO, vertices.data(), MeshPrimitiveUtils::full_byte_size(vertices), bufferUsage);
        MeshPrimitiveUtils::CreateVertexAttrib(m_VAO, m_VBO, attribIndex, 3, MeshPrimitiveUtils::element_byte_size(vertices));
        LOG_INFO("Binding vertices");
    }
    
    if (colours.size() > 0)
    {
        const auto attribIndex = Constants::Renderer::VERTEX_CONSTANTS.AttribIndex.COLOUR;
        MeshPrimitiveUtils::CreateBufferObject(m_CBO, colours.data(), MeshPrimitiveUtils::full_byte_size(colours), bufferUsage);
        MeshPrimitiveUtils::CreateVertexAttrib(m_VAO, m_CBO, attribIndex, 4, MeshPrimitiveUtils::element_byte_size(colours));
        LOG_INFO("Binding colours");
    }

    if (normals.size() > 0)
    {
        const auto attribIndex = Constants::Renderer::VERTEX_CONSTANTS.AttribIndex.NORMAL;
        MeshPrimitiveUtils::CreateBufferObject(m_NBO, normals.data(), MeshPrimitiveUtils::full_byte_size(normals), bufferUsage);
        MeshPrimitiveUtils::CreateVertexAttrib(m_VAO, m_NBO, attribIndex, 3, MeshPrimitiveUtils::element_byte_size(normals));
        LOG_INFO("Binding normals");
    }

    if (texCoords.size() > 0)
    {
        const auto attribIndex = Constants::Renderer::VERTEX_CONSTANTS.AttribIndex.UV0;
        MeshPrimitiveUtils::CreateBufferObject(m_UV0BO, texCoords.data(), MeshPrimitiveUtils::full_byte_size(texCoords), bufferUsage);
        MeshPrimitiveUtils::CreateVertexAttrib(m_VAO, m_UV0BO, attribIndex, 2, MeshPrimitiveUtils::element_byte_size(texCoords));
        LOG_INFO("Binding texCoords");
    }

    if (indices.size() > 0)
    {
        glCreateBuffers(1, &m_IBO);
        glNamedBufferStorage(m_IBO, indices.size() * sizeof(GLuint), indices.data(), bufferUsage);    
        glVertexArrayElementBuffer(m_VAO, m_IBO);
        LOG_INFO("Binding indices");
    }
    glBindVertexArray(m_VAO);
    glBindVertexArray(0);
}

void InstancedMesh::Bind()
{
    glBindVertexArray(m_VAO);
}

void InstancedMesh::Dispatch()
{
    glBindVertexArray(m_VAO);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElementsInstanced(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, nullptr, 1);

    glBindVertexArray(0);

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
