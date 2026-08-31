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

#include "Callbacks/GLFWCallbacks.hpp"
#include "Shader/ShaderLibrary.hpp"
#include "Renderer/RendererConstants.hpp"
#include "Renderer/RendererUtils.hpp"
#include "Logger.hpp"
#include "Loader/MeshLoaders.hpp"
#include "Renderer/Camera.h"
#include "Renderer/Primitives/InstancedMesh.h"

static constexpr int OBJECT_COUNT = 100000;

static GLFWwindow* window;

GLuint program = 0;
GLuint cullProgram = 0;

GLuint objectSSBO;
GLuint indirectBuffer;
GLuint counterBuffer;

GLint vpLocation;
GLint countLocation;
GLint indexCountLocation;

GLint renderVpLocation;

static double previousTime = 0;
static double currentTime = 0;
static double deltaTime = 0;

Camera camera;

InstancedMesh SphereMesh{};

struct Object
{
    glm::mat4 model;
    glm::vec4 position; // xyz = center in local space, w = radius
};

struct DrawElementsIndirectCommand
{
    GLuint count;
    GLuint instanceCount;
    GLuint firstIndex;
    GLuint baseVertex;
    GLuint baseInstance;
};


static void init()
{
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    
    camera = Camera(Perspective,
        1,
        70.0f,
        0.001f,
        1000.0f,
        glm::vec3(0, 0, 4),
        glm::vec3(0, 0, -1));
    
    VertexData data1{};
    MeshLoaders::Static::ImportOBJ(data1, std::string_view("../meshes/surface_sphere.obj"));
    SphereMesh = std::move(data1);
    SphereMesh.Build(true);
    
    program = ShaderLibrary::Load("../shaders/indirect_drawing/indirect_draw.vert","../shaders/indirect_drawing/indirect_draw.frag");
    cullProgram = ShaderLibrary::BuildCompute("../shaders/indirect_drawing/frustumculling.comp");

    std::vector<Object> objects;
    objects.reserve(OBJECT_COUNT);
    for (size_t i=0; i < OBJECT_COUNT; ++i)
    {
        float x =
            float(rand() % 2000 - 1000) * 0.5f;

        float y =
            float(rand() % 2000 - 1000) * 0.5f;

        float z =
            float(rand() % 2000 - 1000) * 0.5f;

        Object obj{};

        obj.model =
            glm::translate(
                glm::mat4(1.0f),
                glm::vec3(x, y, z));

        obj.position =
            glm::vec4(0.0f, 0.0f, 0.0f, 0.866f);

        objects.push_back(obj);
    }

    glGenBuffers(1, &objectSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER,
                 objectSSBO);

    glBufferData(
        GL_SHADER_STORAGE_BUFFER,
        objects.size() * sizeof(Object),
        objects.data(),
        GL_STATIC_DRAW);

    glBindBufferBase(
        GL_SHADER_STORAGE_BUFFER,
        0,
        objectSSBO);

    glGenBuffers(1, &indirectBuffer);

    glBindBuffer(
        GL_DRAW_INDIRECT_BUFFER,
        indirectBuffer);

    glBufferData(
        GL_DRAW_INDIRECT_BUFFER,
        OBJECT_COUNT *
            sizeof(DrawElementsIndirectCommand),
        nullptr,
        GL_DYNAMIC_DRAW);

    glBindBufferBase(
        GL_SHADER_STORAGE_BUFFER,
        1,
        indirectBuffer);


    glGenBuffers(1, &counterBuffer);

    glBindBuffer(
        GL_SHADER_STORAGE_BUFFER,
        counterBuffer);

    unsigned int zero = 0;

    glBufferData(
        GL_SHADER_STORAGE_BUFFER,
        sizeof(unsigned int),
        &zero,
        GL_DYNAMIC_DRAW);

    glBindBufferBase(
        GL_SHADER_STORAGE_BUFFER,
        2,
        counterBuffer);

    vpLocation =
        glGetUniformLocation(
            cullProgram,
            "viewProjection");

    countLocation =
        glGetUniformLocation(
            cullProgram,
            "objectCount");

    indexCountLocation = glGetUniformLocation(
        cullProgram,
        "indexCount");
    
    renderVpLocation =
        glGetUniformLocation(
            program,
            "viewProjection");
}

static void draw()
{
    unsigned int zero = 0;

    glBindBuffer(
        GL_SHADER_STORAGE_BUFFER,
        counterBuffer);

    glBufferSubData(
        GL_SHADER_STORAGE_BUFFER,
        0,
        sizeof(zero),
        &zero);
    
    glUseProgram(cullProgram);

    glm::mat4 viewProjection = camera.projection * camera.view;
    glUniformMatrix4fv(
                vpLocation,
                1,
                GL_FALSE,
                glm::value_ptr(viewProjection));

    glUniform1ui(
        countLocation,
        OBJECT_COUNT);

    glUniform1ui(indexCountLocation, static_cast<GLuint>(SphereMesh.indices.size()));

    glBindBufferBase(
        GL_SHADER_STORAGE_BUFFER,
        0,
        objectSSBO);

    glBindBufferBase(
        GL_SHADER_STORAGE_BUFFER,
        1,
        indirectBuffer);

    glBindBufferBase(
        GL_SHADER_STORAGE_BUFFER,
        2,
        counterBuffer);

    GLuint groups =
        (OBJECT_COUNT + 63) / 64;

    glDispatchCompute(groups, 1, 1);

    //
    // Make compute writes visible to indirect draw.
    //
    glMemoryBarrier(
        GL_COMMAND_BARRIER_BIT |
        GL_SHADER_STORAGE_BARRIER_BIT);

    // ---------------------------------------------------------------------
    // Read number of visible objects
    // ---------------------------------------------------------------------
    //
    // For a real engine you would normally avoid this CPU readback,
    // because it introduces synchronization.
    //
    // This readback is included because it makes the example easy
    // to understand.
    //

    unsigned int visibleCount = 0;

    glBindBuffer(
        GL_SHADER_STORAGE_BUFFER,
        counterBuffer);

    glGetBufferSubData(
        GL_SHADER_STORAGE_BUFFER,
        0,
        sizeof(visibleCount),
        &visibleCount);

    // ---------------------------------------------------------------------
    // Render visible objects
    // ---------------------------------------------------------------------
    
    glClearColor(0.29f, 0.276f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    glUseProgram(program);

    glUniformMatrix4fv(
        renderVpLocation,
        1,
        GL_FALSE,
        glm::value_ptr(viewProjection));

    SphereMesh.Bind();

    glBindBuffer(
        GL_DRAW_INDIRECT_BUFFER,
        indirectBuffer);

    glMultiDrawElementsIndirect(
        GL_TRIANGLES,
        GL_UNSIGNED_INT,
        nullptr,
        visibleCount,
        sizeof(DrawElementsIndirectCommand));
    
    std::printf("\r%u/%d", visibleCount, OBJECT_COUNT);
}

int main()
{
    if (!glfwInit())
    {
        return EXIT_FAILURE;
    }
    
    // glfwWindowHint(GLFW_SAMPLES, 0);
    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
    glfwSetErrorCallback(error_callback);

    window = glfwCreateWindow(800, 800, "OpenGL Indirect Drawing with Frustum Culling", NULL, NULL);
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
    
    glfwSetWindowSizeCallback(window, window_resize);

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
        if (bResizePending)
        {
            bCanRender = false;
            auto [newWidth, newHeight] = RendererStatics::WindowDimensions;
            glViewport(0, 0, newWidth, newHeight);
            camera.aspect_ratio = (float)newWidth / (float)newHeight;
            camera.Update();
            bResizePending = false;
            bCanRender = true;
        }
        if (!bCanRender)
        {
            continue;
        }
        
        draw();
        
        glfwSwapBuffers(window);
        
        currentTime = glfwGetTime();
        deltaTime = currentTime - previousTime;
        previousTime = currentTime;
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    
    return EXIT_SUCCESS;
}