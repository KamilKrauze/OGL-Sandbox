#include "Callbacks/OGLErrorCallbacks.hpp"
#include "Logger.hpp"

#include <array>

#pragma region Debug Data Info

constexpr inline std::array<const char*, 6> SourceNames =
{
    "API",
    "WINDOW_SYS",
    "SHADER_COMPILER",
    "APP",
    "3RDP",
    "MISC",
};

constexpr inline std::array<const char*, 9> TypeNames =
{
    "ERROR",
    "DEPRECATED",
    "UNDEFINED",
    "PORTABILITY",
    "PERFORMANCE",
    "MARKER",
    "PUSH_GROUP",
    "POP_GROUP",
    "MISC",
};

constexpr int32_t BASE_INDEX_SOURCENAME = 0x8246; 
constexpr int32_t BASE_INDEX_TYPENAME = 0x824B;

#pragma endregion

static std::string_view SourceName(GLenum source)
{
    return { SourceNames[source - BASE_INDEX_SOURCENAME] };
}

static std::string_view TypeName(GLenum type)
{
    return {TypeNames[type - BASE_INDEX_TYPENAME] };
}

void GLErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message,
    const void* userParam)
{
    std::string_view sourceName = SourceName(source);
    std::string_view typeName = TypeName(type);
    
    switch (severity)
    {
    case GL_DEBUG_SEVERITY_LOW:
        LOG_INFO("[%s/%s] - %s", sourceName.data(), typeName.data(), message);
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        LOG_WARNING("[%s/%s] - %s", sourceName.data(), typeName.data(), message);
        break;
    case GL_DEBUG_SEVERITY_HIGH:
        LOG_ERROR("[%s/%s] - %s", sourceName.data(), typeName.data(), message);
        break;
    }
}
