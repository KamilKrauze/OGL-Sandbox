#ifndef MESH_PRIMITIVE_UTILS
#define MESH_PRIMITIVE_UTILS

#include <cstdint>
#include <glad/glad.h>

namespace MeshPrimitiveUtils
{
    template<typename ContainerT>
    uint32_t element_byte_size(const ContainerT& data)
    {
        return sizeof(typename ContainerT::value_type);
    }
    
    template<typename ContainerT>
    uint32_t full_byte_size(const ContainerT& data)
    {
        return element_byte_size(data) * data.size();
    }
    
    void CreateBufferObject(uint32_t& bufferObj, const void* data, const uint32_t& data_size, GLenum usage);

    void CreateVertexAttrib(const uint32_t& vao, uint32_t& bufferObj, const uint32_t& attribIndex, const uint16_t components, const uint32_t& data_size);
}

#endif //! MESH_PRIMITIVE_UTILS