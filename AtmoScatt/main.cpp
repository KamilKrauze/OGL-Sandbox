/* Include GL_Load and GLFW headers */
#include <iostream>
#include <ostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

#include "callbacks.hpp"

static GLFWwindow* window;

static void init()
{
    
}

static void draw()
{
    glClearColor(0.29f, 0.276f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

int main()
{
    if (!glfwInit())
    {
        exit(EXIT_FAILURE);
    }
    
    glfwWindowHint(GLFW_SAMPLES, 8);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwSetErrorCallback(error_callback);

    window = glfwCreateWindow(640, 480, "OpenGL Example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to load GLAD" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    {
        int maj_ver, min_ver;
        glGetIntegerv(GL_MAJOR_VERSION, &maj_ver);
        glGetIntegerv(GL_MINOR_VERSION, &min_ver);

        std::cout << "OpenGL Version = " << maj_ver << "." << min_ver << std::endl;

        // Hardware details
        std::cout << "Vender: " << glGetString(GL_VENDOR) << std::endl;
        std::cout << "Version:" << glGetString(GL_VERSION) << std::endl;
        std::cout << "Renderer:" << glGetString(GL_RENDERER) << std::endl;
    }

    init();

    while (!glfwWindowShouldClose(window))
    {
        draw();
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    
    return EXIT_SUCCESS;
}