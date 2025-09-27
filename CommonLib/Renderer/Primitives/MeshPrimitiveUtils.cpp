#include "MeshPrimitiveUtils.h"

void MeshPrimitiveUtils::CreateBufferObject(uint32_t& bufferObj, const void* data, const uint32_t& data_size,
    GLenum usage)
{
    glCreateBuffers(1, &bufferObj);
    glNamedBufferStorage(bufferObj, data_size, data, usage);
}

void MeshPrimitiveUtils::CreateVertexAttrib(const uint32_t& vao, uint32_t& bufferObj, const uint32_t& attribIndex, const uint16_t components,
    const uint32_t& data_size)
{
    
    glEnableVertexArrayAttrib(vao, attribIndex);
    glVertexArrayAttribFormat(vao, attribIndex, components, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vao, attribIndex, attribIndex);
    glVertexArrayVertexBuffer(vao, attribIndex, bufferObj, 0, data_size);
}
