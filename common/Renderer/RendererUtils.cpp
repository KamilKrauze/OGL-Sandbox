#include "RendererUtils.hpp"

#include <iostream>

#include <glad/glad.h>

void RendererUtils::PrintGfxDeviceInfo()
{
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << "\n";
    std::cout << "Graphics Device: " << glGetString(GL_RENDERER) << "\n";
    std::cout << "Device Driver Version: " << glGetString(GL_VERSION) << "\n";
}

void RendererUtils::PrintRendererSpecInfo()
{
    int maj_ver = 0;
    int min_ver = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &maj_ver);
    glGetIntegerv(GL_MINOR_VERSION, &min_ver);

    std::cout << "OpenGL Version: " << maj_ver << "." << min_ver << "\n";
}
