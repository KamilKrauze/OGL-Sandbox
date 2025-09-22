/* Include GL_Load and GLFW headers */
#include <iostream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

#include "Callbacks/GLFWError.hpp"
#include "Shader/ShaderBuilder.hpp"
#include "Renderer/RendererConstants.hpp"
#include "Renderer/RendererUtils.hpp"
#include "Logger.hpp"
#include "GfxBuffers/VertexBuffer.hpp"


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

    Buffers::Vertex::CreateVertexBufferObj<glm::vec3>(vertexBufferObj, 1, verts, GL_STATIC_DRAW);
    Buffers::Vertex::CreateVertexBufferObj<glm::vec4>(colourBufferObj, 1, colors, GL_STATIC_DRAW);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

static void draw()
{
    glClearColor(0.29f, 0.276f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(program);

    Buffers::Vertex::EnableVertexAttribArray(vertexBufferObj,
        Constants::Renderer::VERTEX_CONSTANTS.AttribIndex.POSITION,
        3, GL_FLOAT);
    
    Buffers::Vertex::EnableVertexAttribArray(colourBufferObj,
        Constants::Renderer::VERTEX_CONSTANTS.AttribIndex.COLOUR,
        4, GL_FLOAT);
    
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
    
    // glfwWindowHint(GLFW_SAMPLES, 0);
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