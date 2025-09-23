#ifndef INDEX_BUFFER_HPP
#define INDEX_BUFFER_HPP

#include <glad/glad.h>

namespace Buffers
{
    /**
     * A simple class to abstract basic logic to bind/unbind/delete vertex element buffers using DSA (Direct State Access).
     * @remarks The buffer is designed to not hold any references
     */
    class IndexBufferObject
    {
    public:
        IndexBufferObject() {m_bCanBindBufferData = false;}
        IndexBufferObject(const GLuint& vao_ref) : m_vao(vao_ref) {}
        ~IndexBufferObject();

        /**
         * Assigns the corresponding VAO this buffer is part of.
         * @param vao_ref Vertex Array Object reference
         */
        void GiveVAORef(const GLuint& vao_ref);
        const GLuint& GetVAO();
    
    public:
        /**
         * Creates and sizes the element buffer 
         * @param dataSize The data size in bytes
         * @param data Data block
         * @param usage What kind of drawing (Default = GL_STATIC_DRAW)
         */
        void CreateBuffer(GLsizeiptr dataSize, const void* data, GLenum usage = GL_STATIC_DRAW);
        
        /**
         * Bind vertex element buffer.
         * @remark Less CPU overhead, but less hardware compatible.
         */
        void Bind();

        /**
         * Tries to unbind the element buffer.
         */
        void Unbind();

        /**
         * Unbinds and deletes buffer.
         */
        void Delete();
    private:
        GLuint m_vao = 0;
        GLuint m_indexBO = 0;
        GLenum m_drawUsage = 0;

        bool m_bCanBindBufferData = false;
    };
}

#endif // !INDEX_BUFFER_HPP