#ifndef GLFW_CALLBACKS_HPP
#define GLFW_CALLBACKS_HPP

#include <cstdio>
#include "Logger.hpp"
#include "DelegateHandlers/GeneralSystemEvents.h"
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
//     glViewport(0, 0, width, height);
     // LOG_INFO("[EVENT/WindowResize] - {%d, %d}", width, height);
     // RendererStatics::WindowDimensions = {width, height};
     // WindowResizeEvent.Broadcast(WindowResizePayload(width, height));
//     bResizePending = true;
}

#endif //! GLFW_CALLBACKS_HPP