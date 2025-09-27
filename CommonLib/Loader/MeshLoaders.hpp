#ifndef MESH_LOADERS_HPP
#define MESH_LOADERS_HPP
#include <string_view>

#include "Renderer/Primitives/VertexData.h"


namespace MeshLoaders
{
    namespace Static
    {
        void ImportOBJ(VertexData& vertInfo, const std::string_view filepath);
    }

    namespace Skeletal
    {
        
    }
}

#endif // !#endif MESH_LOADERS_HPP
