#ifndef OGL_ERROR_OUTPUT
#define OGL_ERROR_OUTPUT

#include <glad/glad.h>

void GLAPIENTRY GLErrorCallback(
    GLenum source, GLenum type, GLuint id,
    GLenum severity, GLsizei length, const GLchar* message,
    const void* userParam);

#endif