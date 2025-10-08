/* Include GL_Load and GLFW headers */
#include <array>
#include <chrono>
#include <future>
#include <stack>

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
#include "Loader/MeshLoaders.hpp"
#include "Renderer/Camera.h"
#include "Renderer/RendererStatics.hpp"
#include "Renderer/Primitives/InstancedMesh.h"
#include "Renderer/Texturing/Texture.h"

static GLFWwindow* window;

GLuint surface_shader = 0;
GLuint sky_shader = 0;

InstancedMesh SurfaceMesh{};
Texture Albedo_Map;
Texture ARM_Map; // AO, Roughness, Metallic
Texture Normal_Map; // AO, Roughness, Metallic

InstancedMesh envSky{};
Texture ENV_Texture;

Camera camera;

glm::vec3 translation = glm::vec3(0,0,0);
glm::vec3 rotation = glm::vec3(0 ,glm::radians(45.0), 0);
glm::vec3 scale = glm::vec3(1);
std::stack<glm::mat4> transformStack;
glm::vec3 light_pos = glm::vec3(0,0.2,5);
float light_intensity = 1.0f;

static void init()
{
    // During init, enable debug output
    glEnable              ( GL_DEBUG_OUTPUT );
    glDebugMessageCallback(GLErrorCallback, 0);
    
    camera = Camera(Perspective, 1, 70.0f, 0.001f, 1000000.0f, glm::vec3(0, 0, 4), glm::vec3(0, 0, -1));
    WindowResizeEvent.Bind(&camera, &Camera::UpdateOnWindowResize);
    
    VertexData data1{};
    MeshLoaders::Static::ImportOBJ(data1, std::string_view("../meshes/surface_sphere.obj"));
    SurfaceMesh = std::move(data1);
    SurfaceMesh.Build(true);

    VertexData data2{};
    MeshLoaders::Static::ImportOBJ(data2, std::string_view("../meshes/env_sphere.obj"));
    envSky = std::move(data2);
    envSky.Build();
    
    Albedo_Map .CreateTextureUnit("../textures/surface/aerial_rocks/aerial_rocks_04_diff_2k.jpg",
    TextureSpec(Repeat, Bilinear, Linear,
         GL_RGBA8, GL_RGB, GL_UNSIGNED_BYTE,  true));
    ARM_Map     .CreateTextureUnit("../textures/surface/aerial_rocks/aerial_rocks_04_arm_2k.jpg");
    Normal_Map  .CreateTextureUnit("../textures/surface/aerial_rocks/aerial_rocks_04_nor_gl_2k.jpg",
         TextureSpec(Repeat, Bilinear, Linear,
             GL_RGBA16F, GL_RGB, GL_UNSIGNED_BYTE, true));
    
    ENV_Texture.CreateTextureUnit("../textures/env/hdri/sunflowers_puresky_4k.hdr",
        TextureSpec(Repeat, Linear, Linear, GL_RGB32F, GL_RGB, GL_FLOAT, false));
    
   surface_shader = ShaderBuilder::Load("../shaders/normal_map.vert","../shaders/normal_map.frag");
   sky_shader = ShaderBuilder::Load("../shaders/env_sky.vert","../shaders/env_sky.frag");
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    transformStack.push(glm::mat4(1.0));
}

static double previousTime = 0;
static double currentTime = 0;
static double deltaTime = 0;

float exposure = 1.0f;

static void draw()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.19f, 0.176f, 0.2f, 1.0f);

    glUseProgram(sky_shader);
    glDisable(GL_DEPTH_TEST);
    ENV_Texture.Bind(0);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(sky_shader, "EnvSphereTexture"), 0);
    
    glUniformMatrix4fv(glGetUniformLocation(sky_shader, "view"), 1, GL_FALSE, &camera.view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(sky_shader, "projection"), 1, GL_FALSE, &camera.projection[0][0]);
    glUniform1fv(glGetUniformLocation(sky_shader, "exposure"), 1, &exposure);
    envSky.Dispatch();
    ENV_Texture.Unbind();


    glEnable(GL_DEPTH_TEST);
    
    glUseProgram(surface_shader);
    
    Albedo_Map.Bind(0);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(surface_shader, "AlbedoMap"), 0);

    ARM_Map.Bind(1);
    glActiveTexture(GL_TEXTURE1);
    glUniform1i(glGetUniformLocation(surface_shader, "ARM_Map"), 1);

    Normal_Map.Bind(2);
    glActiveTexture(GL_TEXTURE2);
    glUniform1i(glGetUniformLocation(surface_shader, "NormalMap"), 2);
    transformStack.push(transformStack.top());
    {
        transformStack.top() = glm::translate(transformStack.top(), translation);
        transformStack.top() = glm::rotate(transformStack.top(), rotation.x, glm::vec3(1, 0, 0));
        transformStack.top() = glm::rotate(transformStack.top(), rotation.y, glm::vec3(0, 1, 0));
        transformStack.top() = glm::rotate(transformStack.top(), rotation.z, glm::vec3(0, 0, 1));
        transformStack.top() = glm::scale(transformStack.top(), scale);
        glUniformMatrix4fv(glGetUniformLocation(surface_shader, "model"), 1, GL_FALSE, &transformStack.top()[0][0]);
        glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(camera.view * transformStack.top())));
        glUniformMatrix3fv(glGetUniformLocation(surface_shader, "normal_matrix"), 1, GL_FALSE, value_ptr(normal_matrix));
        glUniformMatrix4fv(glGetUniformLocation(surface_shader, "view"), 1, GL_FALSE, &camera.view[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(surface_shader, "projection"), 1, GL_FALSE, &camera.projection[0][0]);
    }
    glUniform1fv(glGetUniformLocation(surface_shader, "light_intensity"), 1, &light_intensity);
    glUniform3fv(glGetUniformLocation(surface_shader, "LightPos"), 1, &light_pos[0]);
    glUniform3fv(glGetUniformLocation(surface_shader, "CameraPos"), 1, &camera.eye[0]);
    transformStack.pop();
    SurfaceMesh.Dispatch();
    
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
    glfwWindowHint (GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    glfwSetErrorCallback(error_callback);

    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor()); // Valid on GLFW 3.3+ only
    auto& [width, height] = RendererStatics::WindowDimensions;
    window = glfwCreateWindow(width, height, "OpenGL Normal Mapping", NULL, NULL);
    
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
                ImGui::SliderAngle("##rotx", &rotation[0], -360.00f, 360.00f, "%.3f deg");
                ImGui::Text("Rotation Y"); ImGui::SameLine();
                ImGui::SliderAngle("##roty", &rotation[1], -360.00f, 360.00f, "%.3f deg");
                ImGui::Text("Rotation Z"); ImGui::SameLine();
                ImGui::SliderAngle("##rotz", &rotation[2], -360.00f, 360.00f, "%.3f deg");
                ImGui::Separator();
                ImGui::Text("Scale"); ImGui::SameLine();
                ImGui::DragFloat3("##scale", &scale[0], 0.01f,0, 0, "%.3f");
            }
            ImGui::EndChild();

            ImGui::BeginChild("Light Control", ImVec2(0, 100), ImGuiChildFlags_Border);
            {
                ImGui::TextUnformatted("Light Control");
                ImGui::Separator();

                ImGui::Text("Light Intensity"); ImGui::SameLine();
                ImGui::DragFloat("##lit", &light_intensity, 0.001f, 0, 0, "%.4f");
                
                ImGui::Text("Light Position"); ImGui::SameLine();
                ImGui::DragFloat3("##lpos", &light_pos[0], 0.001f, 0, 0, "%.3f");
            }
            ImGui::EndChild();

            
            ImGui::BeginChild("Env Tonemapping", ImVec2(0, 100), ImGuiChildFlags_Border);
            {
                ImGui::TextUnformatted("Env Tonemapping");
                ImGui::Separator();

                ImGui::Text("Exposure"); ImGui::SameLine();
                ImGui::DragFloat("##exp", &exposure, 0.001f, 0, 0, "%.4f");
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

    Albedo_Map.Delete();
    ARM_Map.Delete();
    Normal_Map.Delete();
    SurfaceMesh.Delete();
    glDeleteProgram(surface_shader);
    glDeleteProgram(sky_shader);

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return EXIT_SUCCESS;
}
