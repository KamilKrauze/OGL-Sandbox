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

#include "Callbacks/GLFWCallbacks.hpp"
#include "Shader/ShaderBuilder.hpp"
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

float light_intensity = 1.0f;
float roughness = 1.0f;
float metallic = 0.0f;
float fresnel_coeff = 0.05f;
float fresnel_factor = 1.0f;

glm::vec3 translation = glm::vec3(0,-1,0);
glm::vec3 rotation = glm::vec3(0);
glm::vec3 scale = glm::vec3(1);

GBuffer gbuffer;
static void init()
{
    camera = Camera(Perspective, 1, 45.0f, 0.001f, 1000.0f, glm::vec3(0, 0, 4), glm::vec3(0, 0, -1));
    WindowResizeEvent.Bind(&camera, &Camera::UpdateOnWindowResize);
    WindowResizeEvent.Bind(&gbuffer, &GBuffer::RecreateBuffersOnWindowResize);
    
    VertexData data1{};
    // MeshLoaders::Static::ImportOBJ(data1, std::string_view("../meshes/aston_vantage2018.obj"));
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
    glUniform3fv(glGetUniformLocation(program, "CameraPosition"), 1, &camera.eye[0]);
    glUniform1fv(glGetUniformLocation(program, "shine_factor"), 1, &roughness);
    glUniform1fv(glGetUniformLocation(program, "metallic"), 1, &metallic);
    glUniform1fv(glGetUniformLocation(program, "light_intensity"), 1, &light_intensity);
    glUniform1fv(glGetUniformLocation(program, "fresnel_coeff"), 1, &fresnel_coeff);
    glUniform1fv(glGetUniformLocation(program, "fresnel_factor"), 1, &fresnel_factor);

    quadMesh.Dispatch();
    
    camera.Update();

    // Make sure stack is cleared.
    for (size_t i=0; i <transformStack.size(); ++i)
    {
        transformStack.pop();
    }
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
            gbuffer.RecreateBuffers(newWidth, newHeight);
            camera.aspect_ratio = (float)newWidth / (float)newHeight;
            camera.Update();
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
            ImGui::TextUnformatted("Material");
            ImGui::Separator(); // optional line under the label
        
            ImGui::Text("Roughness"); ImGui::SameLine();
            ImGui::DragFloat("##sf", &roughness, 0.1f, 0, 1, "%.2f");
        
            ImGui::Text("Metallic"); ImGui::SameLine();
            ImGui::DragFloat("##ml", &metallic, 0.01f, 0, 1, "%.2f");

            ImGui::Text("Fresnel Coefficient"); ImGui::SameLine();
            ImGui::DragFloat("##fc", &fresnel_coeff, 0.01f, 0, 0, "%.2f");

            ImGui::Text("Fresnel Factor"); ImGui::SameLine();
            ImGui::DragFloat("##ff", &fresnel_factor, 0.01f, 0, 0, "%.2f");
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
    glDeleteProgram(program);

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return EXIT_SUCCESS;
}
