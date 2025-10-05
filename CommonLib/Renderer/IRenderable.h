#ifndef I_RENDERABLE_H
#define I_RENDERABLE_H

#include <glad\glad.h>

#include "GfxBuffers/IndexBufferObject.hpp"

/**
 * A contract the defines the minimum data and functions required for an object to be drawn to the screen.
 */
class IRenderable
{
public:
    IRenderable() = default;
    IRenderable(IRenderable&& _renderable) noexcept
    {
        m_VAO = _renderable.m_VAO;
    
        m_VBO = _renderable.m_VBO;    
        m_IBO = _renderable.m_IBO;    
   
        m_CBO = _renderable.m_CBO;    
    
        m_UV0BO = _renderable.m_UV0BO;  
        m_UV1BO = _renderable.m_UV1BO; 

        m_NBO = _renderable.m_NBO; 
        m_BNBO = _renderable.m_BNBO; 
        m_TBO = _renderable.m_TBO; 
    }
    virtual ~IRenderable() = default;
public:
    virtual void Build(bool generateMissingNormalData = false) = 0;
    virtual void Bind() = 0;
    virtual void Dispatch() = 0;
    virtual void Unbind() = 0;
    virtual void Delete() = 0;
    
protected:
    GLuint m_VAO = 0;
    
    GLuint m_VBO = 0;   // Vertex position
    GLuint m_IBO = 0;
   
    GLuint m_CBO = 0;   // Vertex colour
    
    GLuint m_UV0BO = 0;  // Vertex texture-coordinates 
    GLuint m_UV1BO = 0;  // Light map coords.

    GLuint m_NBO = 0;
    GLuint m_BNBO = 0;
    GLuint m_TBO = 0;
};

#endif //! I_RENDERABLE_H