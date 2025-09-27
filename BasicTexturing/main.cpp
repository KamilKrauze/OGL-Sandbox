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
#include "GfxBuffers/IndexBufferObject.hpp"
#include "GfxBuffers/VertexBuffer.hpp"
#include "Renderer/RenderList.h"

using Vec2List = std::vector<glm::vec2>;
using Vec3List = std::vector<glm::vec3>;
using Vec4List = std::vector<glm::vec4>;

static GLFWwindow* window;

GLuint program = 0;
GLuint vao = 0;


GLuint posBO = 0;
Vec4List vertPos =
{
    {-0.5,0.5,0,1},
    {0.5,0.5,0,1},  
    {0.5,-0.5,0,1},
    {-0.5,-0.5,0,1},
};

GLuint colourBO = 0;
Vec4List colours =
{
    {1,0,0,0},
    {0,1,0,0},  
    {0,0,1,0},
    {0,0,0,1},
};

Buffers::IndexBufferObject ibo;
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

unsigned int texture0;
unsigned int texture1;

static void CreateTextureUnit(int binding, const char* fp, GLuint& textureObj)
{
    stbi_set_flip_vertically_on_load(true);

    int width, height, channels;
    unsigned char *data = stbi_load(fp, &width, &height, &channels, 3);
    if (data)
    {
        LOG_INFO("{%d, %d} & Channels: %d", width, height, channels);
        glCreateTextures(GL_TEXTURE_2D, 1, &textureObj);

        glTextureStorage2D(textureObj, 1, GL_RGBA16, width, height);
        
        glTextureSubImage2D(textureObj, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateTextureMipmap(textureObj);
        
        glTextureParameteri(textureObj, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(textureObj, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTextureParameteri(textureObj, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTextureParameteri(textureObj, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);

        glBindTextureUnit(binding, textureObj);

        stbi_image_free(data);
    }
    else
    {
        LOG_ERROR("Failed to load texture image");
        stbi_image_free(data);
    }
    glBindTextureUnit(binding,0);
}

static void init()
{
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    program = ShaderBuilder::Load("../shaders/texturing_example.vert","../shaders/texturing_example.frag");
    
    Buffers::Vertex::CreateVertexBufferObj<glm::vec4>(posBO, 1, vertPos, GL_STATIC_DRAW);
    Buffers::Vertex::CreateVertexBufferObj<glm::vec4>(colourBO, 1, colours, GL_STATIC_DRAW);
    Buffers::Vertex::CreateVertexBufferObj<glm::vec2>(uvBO, 1, uv_coords, GL_STATIC_DRAW);
    
    ibo.GiveVAORef(vao);
    ibo.CreateBuffer(sizeof(GLuint) * indices.size(), &indices[0], GL_STATIC_DRAW);
    ibo.Bind();
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    CreateTextureUnit(0, "../textures/rocks_ground_diffuse.png", texture0);
    CreateTextureUnit(1, "../textures/forrest_ground_diffuse.png", texture1);
}

static double previousTime = 0;
static double currentTime = 0;
static double deltaTime = 0;
static void draw()
{
    glClearColor(0.29f, 0.276f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(program);

    glBindTextureUnit(0, texture0);
    glBindTextureUnit(1, texture1);
    
    Buffers::Vertex::EnableVertexAttribArray(posBO,
        Constants::Renderer::VERTEX_CONSTANTS.AttribIndex.POSITION,
        4, GL_FLOAT);

    Buffers::Vertex::EnableVertexAttribArray(posBO,
        Constants::Renderer::VERTEX_CONSTANTS.AttribIndex.COLOUR,
        4, GL_FLOAT);
    
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
    
    glfwWindowHint(GLFW_SAMPLES, 8);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwSetErrorCallback(error_callback);
    
    window = glfwCreateWindow(800, 800, "OpenGL Basic Texturing Example", NULL, NULL);
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
    
    glDeleteBuffers(1, &posBO);
    glDeleteBuffers(1, &colourBO);
    glDeleteVertexArrays(1, &vao);
    glDeleteTextures(1, &texture0);
    glDeleteTextures(1, &texture1);
    glDeleteProgram(program);
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return EXIT_SUCCESS;
}