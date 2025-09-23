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


using Vec3List = std::vector<glm::vec3>;
using Vec4List = std::vector<glm::vec4>;

static GLFWwindow* window;

GLuint program = 0;
GLuint vao = 0;


GLuint vertexBufferObj = 0;
GLuint vertAttrib = 0;
Vec3List verts =
{
    {-0.25,0,0},
    {0.25,0,0},  
    {0.25,-0.5,0},
    {-0.25,-0.5,0},
};

Buffers::IndexBufferObject ibo;
GLuint indexBO = 0;
std::array<GLuint,6> indices
{
    0,1,2,
    2,3,0
};

GLuint colourBufferObj = 0;
GLuint colorAttrib = 1;
Vec4List colors =
{
    {1,0,0,1},
    {0,1,0,1},
    {0,0,1,1},
    {1,1,0,1},
};

GLint posUBID;
glm::vec3 position = {0,0,0};

GLuint SSBO;
struct SSBOLayout
{
    glm::vec4 position = {0,0,0,1};
    glm::vec4 colors = {1,0,0,1};
    
};
std::vector<SSBOLayout> SSBO1 = {
    {{0,0,0,1},{1,0,0,1}},
    {{0,0.55,0,1},{0,1,0,1}}
};

static glm::ivec3 WORK_GROUP_SIZE_MAX;
static GLint WORK_GROUP_INV_MAX;

static void init()
{
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    program = ShaderBuilder::Load("../shaders/simple.vert","../shaders/simple.frag");

    Buffers::Vertex::CreateVertexBufferObj<glm::vec3>(vertexBufferObj, 1, verts, GL_STATIC_DRAW);
    Buffers::Vertex::CreateVertexBufferObj<glm::vec4>(colourBufferObj, 1, colors, GL_STATIC_DRAW);

    ibo.GiveVAORef(vao);
    ibo.CreateBuffer(sizeof(GLuint) * indices.size(), &indices[0], GL_STATIC_DRAW);
    ibo.Bind();
    
    glCreateBuffers(1, &SSBO);
    glNamedBufferStorage(SSBO, sizeof(SSBOLayout) * SSBO1.size(), SSBO1.data(), GL_DYNAMIC_STORAGE_BIT);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    posUBID = glGetUniformLocation(program, "uPosition");

    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &WORK_GROUP_SIZE_MAX.x);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &WORK_GROUP_SIZE_MAX.y);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &WORK_GROUP_SIZE_MAX.z);
    glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &WORK_GROUP_INV_MAX);

    LOG_INFO("Max work group size: {%u,%u,%u}", WORK_GROUP_SIZE_MAX.x, WORK_GROUP_SIZE_MAX.y, WORK_GROUP_SIZE_MAX.z);
    LOG_INFO("Max work group invocations: %d",WORK_GROUP_INV_MAX);
}

static void draw()
{
    glClearColor(0.29f, 0.276f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBO);
    
    glUseProgram(program);
    
    Buffers::Vertex::EnableVertexAttribArray(vertexBufferObj,
        Constants::Renderer::VERTEX_CONSTANTS.AttribIndex.POSITION,
        3, GL_FLOAT);
    
    Buffers::Vertex::EnableVertexAttribArray(colourBufferObj,
        Constants::Renderer::VERTEX_CONSTANTS.AttribIndex.COLOUR,
        4, GL_FLOAT);
    
    glFrontFace(GL_CW);
    glPolygonMode(GL_FRONT, GL_FILL);
    glDrawElementsInstanced(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr, 2);

    position += glm::vec3(0,-0.0001,0);
    glUniform3fv(posUBID, 1, &position[0]);
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

    window = glfwCreateWindow(800, 800, "OpenGL Example", NULL, NULL);
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

    try
    {
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
        }
    }
    catch(std::exception& e)
    {
        throw std::runtime_error(e.what());
    }
    ibo.Delete();

    glfwDestroyWindow(window);
    glfwTerminate();
    
    return EXIT_SUCCESS;
}