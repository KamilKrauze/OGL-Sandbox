/* Include GL_Load and GLFW headers */
#include <array>
#include <chrono>
#include <future>
#include <stack>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include "Callbacks/OGLErrorCallbacks.hpp"
#include "Callbacks/GLFWCallbacks.hpp"
#include "Shader/ShaderBuilder.hpp"
#include "Renderer/RendererUtils.hpp"
#include "Logger.hpp"
#include "GfxBuffers/GBuffer.h"
#include "Loader/MeshLoaders.hpp"
#include "Renderer/Camera.h"
#include "Renderer/RendererStatics.hpp"
#include "Renderer/Primitives/InstancedMesh.h"
#include "Renderer/Texturing/Texture.h"

static GLFWwindow* window;

GLuint program = 0;

InstancedMesh mesh1{};
InstancedMesh quadMesh{};
Camera camera;

Texture diffuseTexture;
Texture ARMTexture; // AO, Roughness, Metallic

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

float light_intensity = 1.0f;
float roughness = 1.0f;
float metallic = 0.0f;
float fresnel_coeff = 0.05f;
float fresnel_factor = 1.0f;

glm::vec3 translation = glm::vec3(0,-0.1,0);
glm::vec3 rotation = glm::vec3(0 ,glm::radians(45.0), 0);
glm::vec3 scale = glm::vec3(1);
std::stack<glm::mat4> transformStack;

GBuffer gbuffer;

static void init()
{
    // During init, enable debug output
    glEnable              ( GL_DEBUG_OUTPUT );
    glDebugMessageCallback(GLErrorCallback, 0);
    
    camera = Camera(Perspective, 1, 70.0f, 0.001f, 1000000.0f, glm::vec3(0, 0, 4), glm::vec3(0, 0, -1));
    WindowResizeEvent.Bind(&camera, &Camera::UpdateOnWindowResize);
    WindowResizeEvent.Bind(&gbuffer, &GBuffer::RecreateBuffersOnWindowResize);
    
    VertexData data1{};
    // MeshLoaders::Static::ImportOBJ(data1, std::string_view("../meshes/aston_vantage2018.obj"));
    MeshLoaders::Static::ImportOBJ(data1, std::string_view("../meshes/cannon_01.obj"));
    mesh1 = std::move(data1);
    mesh1.Build();

    diffuseTexture.CreateTextureUnit(10, "../textures/cannon/cannon_01_diff_4k.jpg");
    ARMTexture.CreateTextureUnit(11, "../textures/cannon/cannon_01_arm_4k.jpg");   
    
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
    
    transformStack.push(glm::mat4(1.0));
    diffuseTexture.Bind(10);
    ARMTexture.Bind(11);

}

static double previousTime = 0;
static double currentTime = 0;
static double deltaTime = 0;

static void draw()
{
    glBindFramebuffer(GL_FRAMEBUFFER, gbuffer.framebuffer);
    // glClearColor(0.19f, 0.176f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE); // ensure depth writes are enabled
    
    glUseProgram(gbuffer.shader);
    transformStack.push(transformStack.top());
    {
        transformStack.top() = glm::translate(transformStack.top(), translation);
        transformStack.top() = glm::rotate(transformStack.top(), rotation.x, glm::vec3(1, 0, 0));
        transformStack.top() = glm::rotate(transformStack.top(), rotation.y, glm::vec3(0, 1, 0));
        transformStack.top() = glm::rotate(transformStack.top(), rotation.z, glm::vec3(0, 0, 1));
        transformStack.top() = glm::scale(transformStack.top(), scale);
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
    gbuffer.ReadBuffer(program);
    glUniform3fv(glGetUniformLocation(program, "CameraPosition"), 1, &camera.eye[0]);
    glUniform1fv(glGetUniformLocation(program, "shine_factor"), 1, &roughness);
    glUniform1fv(glGetUniformLocation(program, "metallic"), 1, &metallic);
    glUniform1fv(glGetUniformLocation(program, "light_intensity"), 1, &light_intensity);
    glUniform1fv(glGetUniformLocation(program, "fresnel_coeff"), 1, &fresnel_coeff);
    glUniform1fv(glGetUniformLocation(program, "fresnel_factor"), 1, &fresnel_factor);

    quadMesh.Dispatch();
    
    camera.Update();
}

int main()
{
    if (!glfwInit())
    {
        return EXIT_FAILURE;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint (GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    glfwSetErrorCallback(error_callback);

    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor()); // Valid on GLFW 3.3+ only
    auto& [width, height] = RendererStatics::WindowDimensions;
    window = glfwCreateWindow(width, height, "OpenGL Deferred Rendering (No Transparency)", NULL, NULL);
    
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

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale * 1.2f;  

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");
    
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
            gbuffer.RecreateBuffers(newWidth, newHeight);
            bResizePending = false;
            bCanRender = true;
        }
        if (!bCanRender)
        {
           continue;
        }
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Scene Controls");
        {

            ImGui::BeginChild("Object Transform", ImVec2(0, 150), ImGuiChildFlags_Border);
            {
                ImGui::Text("Translation"); ImGui::SameLine();
                ImGui::DragFloat3("##trans", &translation[0], 0.01f,0, 0, "%.3f");
                ImGui::Separator();
                ImGui::Text("Rotation X"); ImGui::SameLine();
                ImGui::SliderAngle("##rotx", &rotation[0]);
                ImGui::Text("Rotation Y"); ImGui::SameLine();
                ImGui::SliderAngle("##roty", &rotation[1]);
                ImGui::Text("Rotation Z"); ImGui::SameLine();
                ImGui::SliderAngle("##rotz", &rotation[2]);
                ImGui::Separator();
                ImGui::Text("Scale"); ImGui::SameLine();
                ImGui::DragFloat3("##scale", &scale[0], 0.01f,0, 0, "%.3f");
            }
            ImGui::EndChild();
        
            ImGui::BeginChild("Light Controls", ImVec2(0, 50), ImGuiChildFlags_Border);
            {
                ImGui::Text("Light Intensity"); ImGui::SameLine();
                ImGui::DragFloat("##li", &light_intensity, 0.1f, 0, 0, "%.2f");
            }
            ImGui::EndChild();

            ImGui::BeginChild("Material", ImVec2(0, 150), ImGuiChildFlags_Border);
            {
                ImGui::TextUnformatted("Material");
                ImGui::Separator();
        
                ImGui::Text("Roughness"); ImGui::SameLine();
                ImGui::DragFloat("##sf", &roughness, 0.1f, 0, 1, "%.2f");
        
                ImGui::Text("Metallic"); ImGui::SameLine();
                ImGui::DragFloat("##ml", &metallic, 0.01f, 0, 1, "%.2f");

                ImGui::Text("Fresnel Coefficient"); ImGui::SameLine();
                ImGui::DragFloat("##fc", &fresnel_coeff, 0.01f, 0, 0, "%.2f");

                ImGui::Text("Fresnel Factor"); ImGui::SameLine();
                ImGui::DragFloat("##ff", &fresnel_factor, 0.01f, 0, 0, "%.2f");
            }
            ImGui::EndChild();
        }
        ImGui::End();
    
        draw();

        // Rendering Gui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    
        currentTime = glfwGetTime();
        deltaTime = currentTime - previousTime;
        previousTime = currentTime;
    }

    mesh1.Delete();
    gbuffer.Delete();
    glDeleteProgram(program);

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return EXIT_SUCCESS;
}
