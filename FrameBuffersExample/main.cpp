/* Include GL_Load and GLFW headers */
#include <array>
#include <chrono>
#include <future>
#include <iostream>
#include <stack>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

#include "Callbacks/GLFWCallbacks.hpp"
#include "Shader/ShaderBuilder.hpp"
#include "Renderer/RendererConstants.hpp"
#include "Renderer/RendererUtils.hpp"
#include "Logger.hpp"
#include "GfxBuffers/GBuffer.h"
#include "Loader/MeshLoaders.hpp"
#include "Renderer/Camera.h"
#include "Renderer/RendererStatics.hpp"
#include "Renderer/Primitives/InstancedMesh.h"

static GLFWwindow* window;

GLuint program = 0;

InstancedMesh mesh1{};
InstancedMesh quadMesh{};
Camera camera;

static std::vector<glm::vec3> quadVertices = {
    // positions 
   {-1.0f,  1.0f, 0.0f},
   {-1.0f, -1.0f, 0.0f},
   {1.0f, -1.0f, 0.0f},
   { 1.0f,  1.0f, 0.0f},
};

static std::vector<glm::vec2> quadUVs =
{
    {0.0f, 1.0f},
    {0.0f, 0.0f},
    {1.0f, 0.0f},
    {1.0f, 1.0f},
};

static std::vector<GLuint> quadIndices =
{
    0, 1, 2, 2, 3, 0
};

GBuffer gbuffer;
static void init()
{
    camera = Camera(Perspective, 1, 45.0f, 0.001f, 1000.0f, glm::vec3(0, 0, 4), glm::vec3(0, 0, -1));
    WindowResizeEvent.Bind(&camera, &Camera::UpdateOnWindowResize);
    WindowResizeEvent.Bind(&gbuffer, &GBuffer::RecreateBuffersOnWindowResize);
    
    VertexData data1{};
    MeshLoaders::Static::ImportOBJ(data1, std::string_view("../meshes/SmoothMonkey.obj"));
    mesh1 = std::move(data1);
    mesh1.Build();

    program = ShaderBuilder::Load("../shaders/deferred_rendering/deferred_shader.vert","../shaders/deferred_rendering/deferred_shader.frag");
    gbuffer.shader = ShaderBuilder::Load("../shaders/deferred_rendering/geometry_shader.vert","../shaders/deferred_rendering/geometry_shader.frag");

    quadMesh.vertices = quadVertices;
    quadMesh.texCoords = quadUVs;
    quadMesh.indices = quadIndices;
    quadMesh.Build();

    gbuffer.Create();
    
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
    glBindFramebuffer(GL_FRAMEBUFFER, gbuffer.framebuffer);
    // glClearColor(0.19f, 0.176f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE); // ensure depth writes are enabled
    
    glUseProgram(gbuffer.shader);
    transformStack.push(glm::mat4(1.0));    
    transformStack.push(transformStack.top());
    {
        transformStack.top() = glm::translate(transformStack.top(), glm::vec3(glm::sin((float)currentTime), 0.0f, 0.0f));
        transformStack.top() = glm::rotate(transformStack.top(), -glm::radians(glm::sin((float)currentTime) * 25), glm::vec3(1, 0, 0));
        transformStack.top() = glm::rotate(transformStack.top(), -glm::radians(-glm::cos((float)currentTime) * 50.0f), glm::vec3(0, 1, 0));
        transformStack.top() = glm::rotate(transformStack.top(), -glm::radians(glm::sin((float)currentTime)) * 10.0f, glm::vec3(0, 0, 1));
        transformStack.top() = glm::scale(transformStack.top(), glm::vec3(0.75));
        glUniformMatrix4fv(glGetUniformLocation(gbuffer.shader, "model"), 1, GL_FALSE, &transformStack.top()[0][0]);
        glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(camera.view * transformStack.top())));
        glUniformMatrix3fv(glGetUniformLocation(gbuffer.shader, "normal_matrix"), 1, GL_FALSE, value_ptr(normal_matrix));
        glUniformMatrix4fv(glGetUniformLocation(gbuffer.shader, "view"), 1, GL_FALSE, &camera.view[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(gbuffer.shader, "projection"), 1, GL_FALSE, &camera.projection[0][0]);
        mesh1.Dispatch();
    }
    transformStack.pop();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glUseProgram(program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gbuffer.gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gbuffer.gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gbuffer.gAlbedoSpec);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, gbuffer.gDepth);
    glUniform1i(glGetUniformLocation(program, "gPosition"), 0);
    glUniform1i(glGetUniformLocation(program, "gNormal"), 1);
    glUniform1i(glGetUniformLocation(program, "gAlbedoSpec"), 2);
    glUniform1i(glGetUniformLocation(program, "depth"), 3);
    
    quadMesh.Dispatch();
    
    camera.Update();
}

int main()
{
    if (!glfwInit())
    {
        return EXIT_FAILURE;
    }
    
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwSetErrorCallback(error_callback);

    auto& [width, height] = RendererStatics::WindowDimensions;
    window = glfwCreateWindow(width, height, "OpenGL DSA Drawing Example", NULL, NULL);
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

    mesh1.Delete();
    glDeleteProgram(program);
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return EXIT_SUCCESS;
}
