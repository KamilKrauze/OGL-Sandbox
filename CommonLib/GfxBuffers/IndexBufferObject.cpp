#include "GfxBuffers/IndexBufferObject.hpp"

using namespace Buffers;

IndexBufferObject::~IndexBufferObject()
{
    Delete();
}

void IndexBufferObject::GiveVAORef(const GLuint& vao_ref)
{
    m_bCanBindBufferData = true;
    m_vao = vao_ref;
}

const GLuint& IndexBufferObject::GetVAO()
{
    return m_vao;
}

void IndexBufferObject::CreateBuffer(GLsizeiptr dataSize, const void* data, GLenum usage)
{
    if (!m_bCanBindBufferData) {return;}
    glCreateBuffers(1, &m_indexBO);
    glNamedBufferData(m_indexBO, dataSize, data, usage);

    m_drawUsage = usage;
}

void IndexBufferObject::Bind()
{
    if (!m_bCanBindBufferData) {return;}
    glVertexArrayElementBuffer(m_vao, m_indexBO);
}

void IndexBufferObject::Unbind()
{
    m_drawUsage = 0;
    glVertexArrayElementBuffer(m_vao, 0);
}

void IndexBufferObject::Delete()
{
    Unbind();
    glDeleteBuffers(1, &m_indexBO);
    m_indexBO = 0;
}
    
