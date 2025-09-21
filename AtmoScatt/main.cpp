/* Include GL_Load and GLFW headers */
#include <iostream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

#include "callbacks.hpp"
#include "Logger.hpp"
#include "Shader/ShaderBuilder.hpp"
#include "Renderer/RendererConstants.hpp"
#include "Renderer/RendererUtils.hpp"

using Vec3List = std::vector<glm::vec3>;
using Vec4List = std::vector<glm::vec4>;

static GLFWwindow* window;

GLuint program = 0;
GLuint vao = 0;

GLuint vertexBufferObj = 0;
GLuint vertAttrib = 0;
GLuint colourBufferObj = 0;
GLuint colorAttrib = 1;

Vec3List verts =
{
    {-0.5,0,0},
    {0.5,0,0},  
    {0.5,-0.5,0},  

    {0.5,-0.5,0},
    {-0.5,-0.5,0},
    {-0.5,0,0},
};

Vec4List colors =
{
    {1,0,0,1},
    {0,1,0,1},
    {0,0,1,1},

    {1,1,0,1},
    {0,1,1,1},
    {1,0,1,1},
};

static void init()
{
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    program = ShaderBuilder::Load("../shaders/simple.vert","../shaders/simple.frag");

    glGenBuffers(1, &vertexBufferObj);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * verts.size(), &verts[0], GL_STATIC_DRAW);
    
    glGenBuffers(1, &colourBufferObj);
    glBindBuffer(GL_ARRAY_BUFFER, colourBufferObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * colors.size(), &colors[0], GL_STATIC_DRAW);
}

static void draw()
{
    glClearColor(0.29f, 0.276f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glUseProgram(program);
    
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObj);
    glEnableVertexAttribArray(Constants::Renderer::VERTEX_CONSTANTS.AttribIndex.POSITION);
    glVertexAttribPointer(Constants::Renderer::VERTEX_CONSTANTS.AttribIndex.POSITION, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, colourBufferObj);
    glEnableVertexAttribArray(Constants::Renderer::VERTEX_CONSTANTS.AttribIndex.COLOUR);
    glVertexAttribPointer(Constants::Renderer::VERTEX_CONSTANTS.AttribIndex.COLOUR, 4, GL_FLOAT, GL_FALSE, 0, 0);
    
    glFrontFace(GL_CW);
    glPolygonMode(GL_FRONT, GL_FILL);
    glDrawArraysInstanced(GL_TRIANGLES, 0, verts.size(), 2);
}

int main()
{
    if (!glfwInit())
    {
        return EXIT_FAILURE;
    }
    
    glfwWindowHint(GLFW_SAMPLES, 0);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwSetErrorCallback(error_callback);

    window = glfwCreateWindow(800, 600, "OpenGL Example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to load GLAD" << std::endl;
        exit(EXIT_FAILURE);
    }

    RendererUtils::PrintRendererSpecInfo();
    RendererUtils::PrintGfxDeviceInfo();

    init();

    bool shouldQuit = false;
    while (!shouldQuit)
    {
        if (glfwWindowShouldClose(window))
        {
            shouldQuit = true;
            continue;
        }
        draw();
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    
    return EXIT_SUCCESS;
}