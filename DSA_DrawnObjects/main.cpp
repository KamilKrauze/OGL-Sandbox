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
#include <stack>

#include "stb/stb_image.h"

#include "Callbacks/GLFWCallbacks.hpp"
#include "Shader/ShaderBuilder.hpp"
#include "Renderer/RendererConstants.hpp"
#include "Renderer/RendererUtils.hpp"
#include "Logger.hpp"
#include "Loader/MeshLoaders.hpp"
#include "Renderer/Camera.h"
#include "Renderer/Primitives/InstancedMesh.h"
#include "Renderer/RenderList.h"

using Vec2List = std::vector<glm::vec2>;
using Vec3List = std::vector<glm::vec3>;
using Vec4List = std::vector<glm::vec4>;

static GLFWwindow* window;

GLuint program = 0;

InstancedMesh mesh1{};
InstancedMesh mesh2{};
Camera camera;

static void init()
{
    camera = Camera(Perspective, 1, 45.0f, 0.001f, 1000.0f, glm::vec3(0, 0, 4), glm::vec3(0, 0, -1));
    WindowResizeEvent.Bind(&camera, &Camera::UpdateOnWindowResize);
    
    VertexData data1{};
    VertexData data2{};
    MeshLoaders::Static::ImportOBJ(data1, std::string_view("../meshes/SmoothMonkey.obj"));
    MeshLoaders::Static::ImportOBJ(data2, std::string_view("../meshes/Monkey.obj"));
    mesh1 = std::move(data1);
    mesh2 = std::move(data2);
    program = ShaderBuilder::Load("../shaders/mesh_dsa_draw.vert","../shaders/mesh_dsa_draw.frag");
    
    mesh1.Build();
    mesh2.Build();
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

static double previousTime = 0;
static double currentTime = 0;
static double deltaTime = 0;
std::stack<glm::mat4> transformStack;

static void draw()
{
    glClearColor(0.29f, 0.276f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    
    glUseProgram(program);
    glFrontFace(GL_CW);

    transformStack.push(glm::mat4(1.0));
    transformStack.push(transformStack.top());
    {
        transformStack.top() = glm::translate(transformStack.top(), glm::vec3(-0.75f, 0.0f, 0.0f));
        transformStack.top() = glm::rotate(transformStack.top(), -glm::radians((float)currentTime*2), glm::vec3(1, 0, 0));
        transformStack.top() = glm::rotate(transformStack.top(), -glm::radians(-(float)currentTime*50.0f), glm::vec3(0, 1, 0));
        transformStack.top() = glm::rotate(transformStack.top(), -glm::radians((float)currentTime*5.0f), glm::vec3(0, 0, 1));
        transformStack.top() = glm::scale(transformStack.top(), glm::vec3(1));
        glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, &transformStack.top()[0][0]);
        glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(camera.view * transformStack.top())));
        glUniformMatrix3fv(glGetUniformLocation(program, "normal_matrix"), 1, GL_FALSE, value_ptr(normal_matrix));
        mesh1.Dispatch();
    }
    transformStack.pop();
    
    transformStack.push(transformStack.top());
    {
        transformStack.top() = glm::translate(transformStack.top(), glm::vec3(1.0f, 0.0f, -1.0f));
        transformStack.top() = glm::rotate(transformStack.top(), -glm::radians((float)currentTime*10.0f), glm::vec3(0, 1, 0));
        transformStack.top() = glm::scale(transformStack.top(), glm::vec3(1));
        glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, &transformStack.top()[0][0]);
        glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(camera.view * transformStack.top())));
        glUniformMatrix3fv(glGetUniformLocation(program, "normal_matrix"), 1, GL_FALSE, value_ptr(normal_matrix));
        mesh2.Dispatch();
    }
    transformStack.pop();
    glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, &camera.view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, &camera.projection[0][0]);

    camera.Update();
}

int main()
{
    if (!glfwInit())
    {
        return EXIT_FAILURE;
    }
    
    glfwWindowHint(GLFW_SAMPLES, 8);
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

    mesh2.Delete();
    mesh1.Delete();
    glDeleteProgram(program);
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return EXIT_SUCCESS;
}
