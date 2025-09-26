#ifndef VERTEX_BUFFER_HPP
#define VERTEX_BUFFER_HPP

#include <vector>

#include <glad/glad.h>

namespace Buffers::Vertex
{
    template<typename VecN>
    inline void CreateVertexBufferObj(GLuint& bufferObject, const int vboCount, const std::vector<VecN>& vertex_list, const unsigned int bufferUsage)
    {
        glGenBuffers(vboCount, &bufferObject);
        glBindBuffer(GL_ARRAY_BUFFER, bufferObject);
        glBufferData(GL_ARRAY_BUFFER, sizeof(VecN) * vertex_list.size(), &vertex_list[0], bufferUsage);
    }

    inline void EnableVertexAttribArray(GLuint& buffObj, const unsigned int attribIndex, const int matrixSize, const GLenum dataType, const bool bNormaliseData = false)
    {
        glBindBuffer(GL_ARRAY_BUFFER, buffObj);
        glEnableVertexAttribArray(attribIndex);
        glVertexAttribPointer(attribIndex, matrixSize, dataType, bNormaliseData, 0,0);
    }

    inline void EnableVertexAttribArrayDSA(GLuint& vao, const uint32_t attribIndex, const int components, const GLenum dataType, const bool bNormaliseData = false, const uint32_t offset = 0)
    {
        glEnableVertexArrayAttrib(vao, attribIndex);
        glVertexArrayAttribFormat(vao, attribIndex, components, dataType, bNormaliseData, offset);
        glVertexArrayAttribBinding(vao, attribIndex, 0);
    }
}

#endif //!VERTEX_BUFFER_HPP 