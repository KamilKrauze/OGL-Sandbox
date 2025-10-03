#ifndef GLFW_CALLBACKS_HPP
#define GLFW_CALLBACKS_HPP

#include <cstdio>
#include "Logger.hpp"
#include "Renderer/RendererStatics.hpp"

static bool bResizePending = false;
static bool bCanRender = true;

/* Error callback, outputs error to stl error stream */
static void error_callback(int error, const char* description)
{
    LOG_ERROR("%s", description);
}

/**
 * GLFW callback function when the window is resized.
 * @param window Window reference.
 * @param width  New window width.
 * @param height New window height.
 */
static void window_resize(GLFWwindow* window, int width, int height)
{
    bResizePending = true;
    bCanRender = true;
    RendererStatics::WindowDimensions = {width, height};
}

#endif //! GLFW_CALLBACKS_HPP