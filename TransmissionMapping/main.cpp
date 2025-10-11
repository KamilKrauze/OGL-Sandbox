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
#include "GfxBuffers/TransmissionBuffer.h"
#include "Loader/MeshLoaders.hpp"
#include "Renderer/Camera.h"
#include "Renderer/RendererStatics.hpp"
#include "Renderer/Primitives/InstancedMesh.h"
#include "Renderer/Texturing/Texture.h"

static GLFWwindow* window;

GLuint surface_shader = 0;
GLuint shadow_shader = 0;
GLuint sky_shader = 0;

TransmissionBuffer transmission_buffer{};

InstancedMesh InstancedCannonMesh{};
InstancedMesh PlaneMesh{};

InstancedMesh envSky{};
Texture ENV_Texture;

Camera camera;

glm::vec3 translation = glm::vec3(0,-0.765,0);
glm::vec3 rotation = glm::vec3(0 ,glm::radians(45.0), 0);
glm::vec3 scale = glm::vec3(1.0f);
std::stack<glm::mat4> transformStack;
glm::vec3 light_pos = glm::vec3(2,4,2);
float light_intensity = 1.0f;
float exposure = 1.0f;

static void init()
{
    // During init, enable debug output
    glEnable              ( GL_DEBUG_OUTPUT );
    glDebugMessageCallback(GLErrorCallback, 0);
    
    camera = Camera(Perspective, 1, 70.0f, 0.001f, 1000000.0f, glm::vec3(0, 0, 4), glm::vec3(0, 0, -1));
    
    VertexData data1{};
    MeshLoaders::Static::ImportOBJ(data1, std::string_view("../meshes/cannon_01.obj"));
    InstancedCannonMesh = std::move(data1);
    InstancedCannonMesh.Build();

    VertexData data2{};
    MeshLoaders::Static::ImportOBJ(data2, std::string_view("../meshes/plane.obj"));
    PlaneMesh = std::move(data2);
    PlaneMesh.Build();

    VertexData data3{};
    MeshLoaders::Static::ImportOBJ(data3, std::string_view("../meshes/env_sphere.obj"));
    envSky = std::move(data3);
    envSky.Build();
    
    ENV_Texture.CreateTextureUnit("../textures/env/hdri/sunflowers_puresky_4k.hdr",
        TextureSpec(Repeat, Linear, Linear
            , GL_RGB32F, GL_RGB, GL_FLOAT, false));

    transmission_buffer.shaderProgram = ShaderBuilder::Load(
        "../shaders/transmission_mapping/transmission_mapper.vert",
        "../shaders/transmission_mapping/transmission_mapper.frag"
    );
    
    surface_shader = ShaderBuilder::Load(
        "../shaders/transmission_mapping/surface.vert",
        "../shaders/transmission_mapping/surface.frag"
        );
    sky_shader = ShaderBuilder::Load("../shaders/env_sky.vert","../shaders/env_sky.frag");

    transmission_buffer.Create();
    
    transformStack.push(glm::mat4(1.0));
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

float lrtb = 5.0f;

static double previousTime = 0;
static double currentTime = 0;
static double deltaTime = 0;

static void draw()
{
    transmission_buffer.Bind();
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);  // additive blend (accumulate RGB)
    glBlendEquation(GL_FUNC_ADD);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Shadow mapping
    glUseProgram(transmission_buffer.shaderProgram);
    transmission_buffer.Update(light_pos);

    // Draw scene from light POV
    {
        // plane
        glDisable(GL_CULL_FACE);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0,-0.75,0));
        model = glm::scale(model, glm::vec3(25.0f));
        glUniform4fv(glGetUniformLocation(transmission_buffer.shaderProgram, "TranslucentColour"), 1, glm::value_ptr(glm::vec4(1,1,1,1)));
        glUniformMatrix4fv(glGetUniformLocation(transmission_buffer.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        PlaneMesh.Dispatch();
        glEnable(GL_CULL_FACE);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1,0,1));
        model = glm::rotate(model, -glm::radians(-90.0f), glm::vec3(1,0,0));
        model = glm::rotate(model, -glm::radians(0.0f), glm::vec3(0,1,0));
        model = glm::rotate(model, -glm::radians(-45.0f), glm::vec3(0,0,1));
        model = glm::scale(model, glm::vec3(0.25f));
        glUniform4fv(glGetUniformLocation(transmission_buffer.shaderProgram, "TranslucentColour"), 1, glm::value_ptr(glm::vec4(1,1,0,0.6)));
        glUniformMatrix4fv(glGetUniformLocation(transmission_buffer.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        PlaneMesh.Dispatch();
        
        model = glm::mat4(1.0f);
        model = glm::translate(model, translation);
        model = glm::rotate(model, rotation.y, glm::vec3(0,1,0));
        model = glm::scale(model, scale);
        glUniform4fv(glGetUniformLocation(transmission_buffer.shaderProgram, "TranslucentColour"), 1, glm::value_ptr(glm::vec4(1,0,0,1)));
        glUniformMatrix4fv(glGetUniformLocation(transmission_buffer.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        InstancedCannonMesh.Dispatch();
    }
    transmission_buffer.Unbind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Environmental sky
    glUseProgram(sky_shader);
    glDisable(GL_DEPTH_TEST);
    ENV_Texture.Bind(0);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(sky_shader, "EnvSphereTexture"), 0);
    transformStack.push(transformStack.top());
    {
        transformStack.top() = glm::scale(transformStack.top(), glm::vec3(5.0f));
        glUniformMatrix4fv(glGetUniformLocation(sky_shader, "model"), 1, GL_FALSE, &transformStack.top()[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(sky_shader, "view"), 1, GL_FALSE, &camera.view[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(sky_shader, "projection"), 1, GL_FALSE, &camera.projection[0][0]);
        glUniform1fv(glGetUniformLocation(sky_shader, "exposure"), 1, &exposure);
    }
    envSky.Dispatch();
    transformStack.pop();
    ENV_Texture.Unbind();
    
    glEnable(GL_DEPTH_TEST);
    
    glUseProgram(surface_shader);
    transmission_buffer.BindData(surface_shader, 0, 1);
    glUniform1fv(glGetUniformLocation(surface_shader, "pcf_kernel_width"), 1, &transmission_buffer.pcf_kernel_width);
    // Plane transform
    transformStack.push(transformStack.top());
    {
        transformStack.top() = glm::translate(transformStack.top(), glm::vec3(0,-0.75,0));
        transformStack.top() = glm::rotate(transformStack.top(), -glm::radians(0.0f), glm::vec3(1, 0, 0));
        transformStack.top() = glm::rotate(transformStack.top(), 0.0f, glm::vec3(0, 1, 0));
        transformStack.top() = glm::rotate(transformStack.top(), 0.0f, glm::vec3(0, 0, 1));
        transformStack.top() = glm::scale(transformStack.top(), glm::vec3(5.0));

        glUniformMatrix4fv(glGetUniformLocation(surface_shader, "model"), 1, GL_FALSE, &transformStack.top()[0][0]);
        glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(camera.view * transformStack.top())));
        glUniformMatrix3fv(glGetUniformLocation(surface_shader, "normal_matrix"), 1, GL_FALSE, value_ptr(normal_matrix));
        glUniformMatrix4fv(glGetUniformLocation(surface_shader, "view"), 1, GL_FALSE, &camera.view[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(surface_shader, "projection"), 1, GL_FALSE, &camera.projection[0][0]);
    }
    PlaneMesh.Dispatch();
    transformStack.pop();

    // Plane transform
    transformStack.push(transformStack.top());
    {
        transformStack.top() = glm::translate(transformStack.top(), glm::vec3(-1,0,1));
        transformStack.top() = glm::rotate(transformStack.top(), -glm::radians(-90.0f), glm::vec3(1, 0, 0));
        transformStack.top() = glm::rotate(transformStack.top(), -glm::radians(-0.0f), glm::vec3(0, 1, 0));
        transformStack.top() = glm::rotate(transformStack.top(), -glm::radians(-45.0f), glm::vec3(0, 0, 1));
        transformStack.top() = glm::scale(transformStack.top(), glm::vec3(0.25));

        glUniformMatrix4fv(glGetUniformLocation(surface_shader, "model"), 1, GL_FALSE, &transformStack.top()[0][0]);
        glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(camera.view * transformStack.top())));
        glUniformMatrix3fv(glGetUniformLocation(surface_shader, "normal_matrix"), 1, GL_FALSE, value_ptr(normal_matrix));
        glUniformMatrix4fv(glGetUniformLocation(surface_shader, "view"), 1, GL_FALSE, &camera.view[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(surface_shader, "projection"), 1, GL_FALSE, &camera.projection[0][0]);
    }
    PlaneMesh.Dispatch();
    transformStack.pop();

    // Sphere transform
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
    InstancedCannonMesh.Dispatch();
    transformStack.pop();
    
    glUniform1fv(glGetUniformLocation(surface_shader, "light_intensity"), 1, &light_intensity);
    glUniform3fv(glGetUniformLocation(surface_shader, "LightPos"), 1, &light_pos[0]);
    glUniform3fv(glGetUniformLocation(surface_shader, "CameraPosition"), 1, &camera.eye[0]);
    
    camera.Update();
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
    glfwWindowHint (GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    glfwSetErrorCallback(error_callback);

    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor()); // Valid on GLFW 3.3+ only
    auto& [width, height] = RendererStatics::WindowDimensions;
    window = glfwCreateWindow(width, height, "OpenGL Transmission Mapping Experiment", NULL, NULL);
    
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
                ImGui::DragFloat3("##lpos", &light_pos[0], 0.01f, 0, 0, "%.2f");
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

        ImGui::Begin("Crappy Camera Controls");
        {
            ImGui::Columns(2, NULL);
            ImGui::SetColumnWidth(0, 100);
            ImGui::Separator();
            
            ImGui::Text("Eye"); ImGui::SameLine();
            ImGui::NextColumn();
            {
                ImGui::PushItemWidth(75.0f);
                ImGui::TextColored({1,0,0,1}, "X");
                ImGui::SameLine();
                ImGui::DragFloat("##eye_x", &camera.eye[0], 0.01f,0, 0, "%.2f");
                ImGui::SameLine();
                ImGui::TextColored({0,1,0,1}, "Y");
                ImGui::SameLine();
                ImGui::DragFloat("##eye_y", &camera.eye[1], 0.01f,0, 0, "%.2f");
                ImGui::SameLine();
                ImGui::TextColored({0,0,1,1}, "Z");
                ImGui::SameLine();
                ImGui::DragFloat("##eye_z", &camera.eye[2], 0.01f,0, 0, "%.2f");
                ImGui::PopItemWidth();
            }
            ImGui::NextColumn();
            ImGui::Separator();
            
            ImGui::Text("Centre"); ImGui::SameLine();
            ImGui::NextColumn();
            {
                ImGui::PushItemWidth(75.0f);
                ImGui::TextColored({1,0,0,1}, "X");
                ImGui::SameLine();
                ImGui::DragFloat("##centre_x", &camera.centre[0], 0.01f,0, 0, "%.2f");
                ImGui::SameLine();
                ImGui::TextColored({0,1,0,1}, "Y");
                ImGui::SameLine();
                ImGui::DragFloat("##centre_y", &camera.centre[1], 0.01f,0, 0, "%.2f");
                ImGui::SameLine();
                ImGui::TextColored({0,0,1,1}, "Z");
                ImGui::SameLine();
                ImGui::DragFloat("##centre_z", &camera.centre[2], 0.01f,0, 0, "%.2f");
                ImGui::PopItemWidth();
            }
        }
        ImGui::End();

        ImGui::Begin("Crappy Shadow Controls");
        {
            ImGui::Image((ImTextureID)(intptr_t)transmission_buffer.shadowMap,
                ImVec2(256, 256), ImVec2(0,1), ImVec2(1,0));
            ImGui::SameLine();
            ImGui::Image((ImTextureID)(intptr_t)transmission_buffer.transmissionMap,
                ImVec2(256, 256), ImVec2(0,1), ImVec2(1,0));
            
            ImGui::Text("PCF Kernel Width"); ImGui::SameLine();
            ImGui::DragFloat("##pcf", &transmission_buffer.pcf_kernel_width, 1.0f, 0, 0, "%.1f");
            
            ImGui::Text("Ortho Margin"); ImGui::SameLine();
            ImGui::DragFloat("##ortho", &transmission_buffer.projection_fov, 0.01f, 0, 0, "%.2f");

            ImGui::Text("Light Near Plane"); ImGui::SameLine();
            ImGui::DragFloat("##near", &transmission_buffer.near_plane, 0.01f, 0, 0, "%.2f");

            ImGui::Text("Light Far Plane"); ImGui::SameLine();
            ImGui::DragFloat("##far", &transmission_buffer.far_plane, 0.01f, 0, 0, "%.2f"); 
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

    ENV_Texture.Delete();
    envSky.Delete();
    InstancedCannonMesh.Delete();
    PlaneMesh.Delete();
    
    glDeleteProgram(shadow_shader);
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
