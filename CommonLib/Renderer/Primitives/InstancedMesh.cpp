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
    bitangents  = std::move(data.binormals);

    return *this;
}

void InstancedMesh::Build(bool generateMissingNormalData)
{
    glCreateVertexArrays(1, &m_VAO);
    if (vertices.size() > 0)
    {
        const auto attribIndex = Constants::Renderer::VERTEX_CONSTANTS.AttribIndex.POSITION;
        MeshPrimitiveUtils::CreateBufferObject(m_VBO, vertices.data(), MeshPrimitiveUtils::full_byte_size(vertices), bufferUsage);
        MeshPrimitiveUtils::CreateVertexAttrib(m_VAO, m_VBO, attribIndex, 3, MeshPrimitiveUtils::element_byte_size(vertices));
    }
    
    if (colours.size() > 0)
    {
        const auto attribIndex = Constants::Renderer::VERTEX_CONSTANTS.AttribIndex.COLOUR;
        MeshPrimitiveUtils::CreateBufferObject(m_CBO, colours.data(), MeshPrimitiveUtils::full_byte_size(colours), bufferUsage);
        MeshPrimitiveUtils::CreateVertexAttrib(m_VAO, m_CBO, attribIndex, 4, MeshPrimitiveUtils::element_byte_size(colours));
    }

    if (normals.size() > 0)
    {
        auto attribIndex = Constants::Renderer::VERTEX_CONSTANTS.AttribIndex.NORMAL;
        MeshPrimitiveUtils::CreateBufferObject(m_NBO, normals.data(), MeshPrimitiveUtils::full_byte_size(normals), bufferUsage);
        MeshPrimitiveUtils::CreateVertexAttrib(m_VAO, m_NBO, attribIndex, 3, MeshPrimitiveUtils::element_byte_size(normals));

        if (generateMissingNormalData)
        {
            GenerateTangentsAndBiNormals();
            attribIndex = Constants::Renderer::VERTEX_CONSTANTS.AttribIndex.TANGENT;
            MeshPrimitiveUtils::CreateBufferObject(m_TBO, tangents.data(), MeshPrimitiveUtils::full_byte_size(tangents), bufferUsage);
            MeshPrimitiveUtils::CreateVertexAttrib(m_VAO, m_TBO, attribIndex, 3, MeshPrimitiveUtils::element_byte_size(tangents));

            attribIndex = Constants::Renderer::VERTEX_CONSTANTS.AttribIndex.BITANGENTS;
            MeshPrimitiveUtils::CreateBufferObject(m_BNBO, bitangents.data(), MeshPrimitiveUtils::full_byte_size(bitangents), bufferUsage);
            MeshPrimitiveUtils::CreateVertexAttrib(m_VAO, m_BNBO, attribIndex, 3, MeshPrimitiveUtils::element_byte_size(bitangents));
        }
    }

    if (texCoords.size() > 0)
    {
        const auto attribIndex = Constants::Renderer::VERTEX_CONSTANTS.AttribIndex.UV0;
        MeshPrimitiveUtils::CreateBufferObject(m_UV0BO, texCoords.data(), MeshPrimitiveUtils::full_byte_size(texCoords), bufferUsage);
        MeshPrimitiveUtils::CreateVertexAttrib(m_VAO, m_UV0BO, attribIndex, 2, MeshPrimitiveUtils::element_byte_size(texCoords));
    }

    if (indices.size() > 0)
    {
        glCreateBuffers(1, &m_IBO);
        glNamedBufferStorage(m_IBO, indices.size() * sizeof(GLuint), indices.data(), bufferUsage);    
        glVertexArrayElementBuffer(m_VAO, m_IBO);
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

void InstancedMesh::GenerateTangentsAndBiNormals()
{
    tangents.assign(vertices.size(), glm::vec3(0.0f));
    bitangents.assign(vertices.size(), glm::vec3(0.0f));
    // Loop through all triangles
    for (size_t i=0; i < indices.size(); i+=3)
    {
        const uint32_t i0 = indices[i + 0];
        const uint32_t i1 = indices[i + 1];
        const uint32_t i2 = indices[i + 2];

        const glm::vec3& pos0 = vertices[i0];
        const glm::vec3& pos1 = vertices[i1];
        const glm::vec3& pos2 = vertices[i2];

        const glm::vec2& uv0 = texCoords[i0];
        const glm::vec2& uv1 = texCoords[i1];
        const glm::vec2& uv2 = texCoords[i2];

        const glm::vec3 e1 = pos1 - pos0;
        const glm::vec3 e2 = pos2 - pos0;

        const glm::vec2 duv1 = uv1 - uv0;
        const glm::vec2 duv2 = uv2 - uv0;
        
        float det = (duv1.x * duv2.y) - (duv1.y * duv2.x);
        float r = (fabs(det) < 1e-6f) ? 1.0f : 1.0f / det;

        const glm::vec3 tangent = (e1 * duv2.y - e2 * duv1.x) * r;
        const glm::vec3 binormal = (e2 * duv1.x - e1 * duv2.x) * r;

        tangents[i0] += tangent;
        tangents[i1] += tangent;
        tangents[i2] += tangent;

        bitangents[i0] += binormal;
        bitangents[i1] += binormal;
        bitangents[i2] += binormal;
    }
    // Normalize and orthogonalize
    for (size_t i=0; i < vertices.size(); ++i)
    {
        glm::vec3 n = glm::normalize(normals[i]);
        glm::vec3 t = tangents[i];
        // Gram-Schmidt orthogonalization
        tangents[i] = glm::normalize(t - n * glm::dot(n,t));
        // Calculate handedness for bitangent
        const float handedness = (glm::dot(glm::cross(n, tangents[i]), bitangents[i]) < 0.0f) ? -1.0f : 1.0f;
        bitangents[i] = handedness * glm::normalize(glm::cross(n, tangents[i]));
    }
}
