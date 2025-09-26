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

#include "Callbacks/GLFWError.hpp"
#include "Shader/ShaderBuilder.hpp"
#include "Renderer/RendererConstants.hpp"
#include "Renderer/RendererUtils.hpp"
#include "Logger.hpp"
#include "GfxBuffers/IndexBufferObject.hpp"
#include "GfxBuffers/VertexBuffer.hpp"


using Vec2List = std::vector<glm::vec2>;
using Vec3List = std::vector<glm::vec3>;
using Vec4List = std::vector<glm::vec4>;

static GLFWwindow* window;

GLuint compute_program = 0;
GLuint program = 0;
GLuint vao = 0;


GLuint vertexBufferObj = 0;
Vec3List verts =
{
    {-0.5,0.5,0},
    {0.5,0.5,0},  
    {0.5,-0.5,0},
    {-0.5,-0.5,0},
};

Buffers::IndexBufferObject ibo;
GLuint indexBO = 0;
std::array<GLuint,6> indices
{
    0,1,2,
    2,3,0
};

GLuint uvBO = 0;
Vec2List uv_coords =
{
    {0,0},
    {1,0},
    {1,1},
    {0,1},
};

static glm::ivec3 WORK_GROUP_SIZE_MAX;
static GLint WORK_GROUP_INV_MAX;

static const uint32_t MAX_WIDTH = 1024;
static const uint32_t MAX_HEIGHT = 1024;

unsigned int texture;


static void init()
{
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    program = ShaderBuilder::Load("../shaders/simple_comp.vert","../shaders/simple_comp.frag");
    compute_program = ShaderBuilder::BuildCompute("../shaders/simple_comp.comp");
    
    Buffers::Vertex::CreateVertexBufferObj<glm::vec3>(vertexBufferObj, 1, verts, GL_STATIC_DRAW);
    Buffers::Vertex::CreateVertexBufferObj<glm::vec2>(uvBO, 1, uv_coords, GL_STATIC_DRAW);

    ibo.GiveVAORef(vao);
    ibo.CreateBuffer(sizeof(GLuint) * indices.size(), &indices[0], GL_STATIC_DRAW);
    ibo.Bind();
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    int a = 0;
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &WORK_GROUP_SIZE_MAX.x);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &WORK_GROUP_SIZE_MAX.y);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &WORK_GROUP_SIZE_MAX.z);
    glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &WORK_GROUP_INV_MAX);

    LOG_INFO("Max work group size: {%u,%u,%u}", WORK_GROUP_SIZE_MAX.x, WORK_GROUP_SIZE_MAX.y, WORK_GROUP_SIZE_MAX.z);
    LOG_INFO("Max work group invocations: %u",WORK_GROUP_INV_MAX);

    glUseProgram(program);
    
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, MAX_WIDTH, MAX_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);

    glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
}

std::vector<float> px(4 * MAX_WIDTH * MAX_HEIGHT);

static double previousTime = 0;
static double currentTime = 0;
static double deltaTime = 0;
static void draw()
{
    glUseProgram(compute_program);
    float timeConst = glfwGetTime();
    glUniform1f(glGetUniformLocation(compute_program, "TIME"), timeConst);
    glUniform1f(glGetUniformLocation(compute_program, "DELTA_TIME"), deltaTime);
    glDispatchCompute(MAX_WIDTH/16, MAX_HEIGHT/16, 1);
    // Make sure writing is complete before other shaders read.
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    glClearColor(0.29f, 0.276f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program, "tex"), 0);
    glUniform1f(glGetUniformLocation(compute_program, "TIME"), glfwGetTime());


    
    Buffers::Vertex::EnableVertexAttribArray(vertexBufferObj,
        Constants::Renderer::VERTEX_CONSTANTS.AttribIndex.POSITION,
        3, GL_FLOAT);
    
    Buffers::Vertex::EnableVertexAttribArray(uvBO,
        Constants::Renderer::VERTEX_CONSTANTS.AttribIndex.UV0,
        2, GL_FLOAT);
    
    glFrontFace(GL_CW);
    glPolygonMode(GL_FRONT, GL_FILL);
    glDrawElementsInstanced(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr, 1);
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

    window = glfwCreateWindow(800, 800, "OpenGL Basic Compute R/W Shader Example", NULL, NULL);
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
    ibo.Delete();

    glfwDestroyWindow(window);
    glfwTerminate();
    
    return EXIT_SUCCESS;
}