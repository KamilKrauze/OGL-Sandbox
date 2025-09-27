/* Include GL_Load and GLFW headers */
#include <array>
#include <iostream>
#include <vector>
#include <chrono>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "stb/stb_image.h"

#include "Callbacks/GLFWCallbacks.hpp"
#include "Shader/ShaderBuilder.hpp"
#include "Renderer/RendererConstants.hpp"
#include "Renderer/RendererUtils.hpp"
#include "Logger.hpp"
#include "Loader/MeshLoaders.hpp"
#include "Renderer/Primitives/InstancedMesh.h"
#include "Renderer/RenderList.h"

using Vec2List = std::vector<glm::vec2>;
using Vec3List = std::vector<glm::vec3>;
using Vec4List = std::vector<glm::vec4>;

static GLFWwindow* window;

GLuint program = 0;

InstancedMesh mesh{};

static void init()
{
    VertexData data{};
    MeshLoaders::Static::ImportOBJ(data, std::string_view("../meshes/Monkey.obj"));
    mesh = std::move(data);
    program = ShaderBuilder::Load("../shaders/mesh_dsa_draw.vert","../shaders/mesh_dsa_draw.frag");
    
    mesh.Build();
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

static double previousTime = 0;
static double currentTime = 0;
static double deltaTime = 0;
static void draw()
{
    glClearColor(0.29f, 0.276f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    glUseProgram(program);
    // mesh.Bind();
    glFrontFace(GL_CW);
    mesh.Dispatch();
    // mesh.Unbind();
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
    
    window = glfwCreateWindow(800, 800, "OpenGL DSA Drawing Example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return EXIT_FAILURE;
    }
    
    glfwMakeContextCurrent(window);
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        LOG_ERROR("Failed to initialize GLAD");
        return EXIT_FAILURE;
    }
    
    glfwSetFramebufferSizeCallback(window, window_resize);
    
    RendererUtils::PrintRendererSpecInfo();
    RendererUtils::PrintGfxDeviceInfo();
    
    init();
    bool shouldQuit = false;

    previousTime = glfwGetTime();
    while (!shouldQuit)
    {
        glfwPollEvents();
        if (glfwWindowShouldClose(window))
        {
            shouldQuit = true;
            continue;
        }
        draw();
    
        glfwSwapBuffers(window);
    
        currentTime = glfwGetTime();
        deltaTime = currentTime - previousTime;
        previousTime = currentTime;
    }

    mesh.Delete();
    glDeleteProgram(program);
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return EXIT_SUCCESS;
}
