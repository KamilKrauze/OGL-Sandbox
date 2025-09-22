#ifndef RENDERER_CONSTANTS_HPP
#define RENDERER_CONSTANTS_HPP
#include <cstdint>

namespace Constants::Renderer
{
    struct VertexConstants {
        union {
            struct {
                uint32_t POSITION;
                uint32_t COLOUR;
                uint32_t NORMAL;
                uint32_t TANGENT;
                uint32_t BINORMAL;
                uint32_t UV;
                uint32_t CUSTOM1;
                uint32_t CUSTOM2;
            } AttribIndex;
        };

        // constexpr constructor for compile-time initialization
        constexpr VertexConstants()
            : AttribIndex{0, 1, 2, 3, 4, 5, 6, 7} {}
    };

    // static storage, compile-time constant
    static constexpr VertexConstants VERTEX_CONSTANTS{};

}

#endif