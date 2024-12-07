#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include "camera.h"
#include "menu.h"
#include "model.h"
#include "mesh.h"
#include <GL/glut.h>
#include <stdio.h>
#include <imgui.h>
#include <fstream>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#define GL_SILENCE_DEPRECATION

GLFWwindow* window;

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 800;

// Global values
float last_x, last_y;
bool first_mouse = true;
float cameraSpeed = 5.f;

Camera camera;
Menu menu(camera);
Model model;

//Controls
void process_keypresses(GLFWwindow* window, float deltaTime);
void mouse_callback(GLFWwindow* window, double x_pos, double y_pos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

//Check file
inline bool file_exists(const std::string& name);

int main(int* argc, char** argv)
{
    try {
        std::string ModelPath = "./models/rat.stl";

        if (file_exists(ModelPath) == false) {
            std::cout << "Default file not found" << std::endl;
            return 0;
        }
        if (!glfwInit())
            return -1;

        glfwWindowHint(GLFW_SAMPLES, 4);
        // We need to explicitly ask for a 4.1, core profile context on OS X
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // Open fullscreen or windowed context based on config.ini
        GLFWmonitor* monitor;
        int window_width, window_height = 0;
        window_width = SCR_WIDTH;
        window_height = SCR_HEIGHT;
        GLFWwindow* window = glfwCreateWindow(window_width, window_height, "STL Model Viewer", NULL, NULL);
        camera.setScreenDimensions(window_width, window_height);

        // Make the window's context current
        glfwMakeContextCurrent(window);
        // Set input callbacks
        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetScrollCallback(window, scroll_callback);
        // Set vsync
        glfwSwapInterval(1);

        // Setup Dear ImGui context
        const char* glsl_version = "#version 130";
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        //ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);
        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        if (!window)
        {
            glfwTerminate();
            return -1;
        }

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return -1;
        }

        glEnable(GL_DEPTH_TEST);   // Depth testing
        glEnable(GL_CULL_FACE);    // Rear face culling
        glEnable(GL_MULTISAMPLE);  // MSAA

        // Set up input sensitivities
        float* mouse_sensitivity = &menu.getMouseSensitivity();
        float* zoom_sensitivity = &menu.getZoomSensitivity();
        float* fov_sensitivity = &menu.getFovSensitivity();
        camera.setSensitivities(*mouse_sensitivity, *zoom_sensitivity, *fov_sensitivity);

        // Instantiate, compile and link shader
        Shader shader(
            "shaders/vertex.glsl",
            "shaders/fragment.glsl",
            "shaders/geometry.glsl"
        );
        shader.use(); // There is only have one shader so this one can remain attached

        //Load a default 3DModel from the default path
        model = model.load3DModel(ModelPath, shader, &menu);
        //Time and Frame Animation
        float deltaTime, currentFrame, lastFrame = 0;
        bool show_demo_window = true;
        bool show_another_window = false;

        //We store float[] values that are then affected then transferred to glm menu values
        //BackGround and Scene Temporary Values
        float* backGroundColorTmp = glm::value_ptr(menu.getBackgroundColor());

        //Material Temporary Values
        float* ambientMaterialColorTmp = glm::value_ptr(menu.getAmbientMaterialColor());
        float* diffuseMaterialColorTmp = glm::value_ptr(menu.getDiffuseMaterialColor());
        float* specularMaterialColorTmp = glm::value_ptr(menu.getSpecularMaterialColor());

        //Scene Lighting Temporary Values
        float* ambientLightingColorTmp = glm::value_ptr(menu.getAmbientLightingColor());
        float* diffuseLightingColorTmp = glm::value_ptr(menu.getDiffuseLightingColor());
        float* specularLightingColorTmp = glm::value_ptr(menu.getSpecularLightingColor());

        //Path Temporary value
        char* modelPathTmp = ModelPath.data();
        shader.reloadValues(&menu);
        while (!glfwWindowShouldClose(window))
        {
            //Create ImGui Frames
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            // Delta time calculations
            float currentFrame = glfwGetTime();
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            ImGui::Begin("3D Object Viewer");                          // Create a window called "Hello, world!" and append into it.

            //Menu Bar
            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem("Create")) {
                        /*show_another_window = true;
                        ImGui::Begin("Window A", &show_another_window);
                        //ImGui::Text("Here");
                        ImGui::End();*/
                    }
                    if (ImGui::MenuItem("Open", "Ctrl+O")) {
                    }
                    if (ImGui::MenuItem("Save", "Ctrl+S")) {
                    }
                    if (ImGui::MenuItem("Save as..")) {
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }
            //Model Path and name
            ImGui::InputText("Model", modelPathTmp, 50, 0, NULL, NULL);
            if (ImGui::Button("Load")) {
                if (file_exists(modelPathTmp) == false) {
                    //ImGui::BeginPopupModal("Error", NULL, ImGuiWindowFlags_AlwaysAutoResize);
                    ImGui::OpenPopup("Message Box", ImGuiPopupFlags_NoReopen);
                    ImGui::Text("STL Model not found. Make sure the file is in the models folder");
                    ImGui::Separator();
                    //ImGui::CloseCurrentPopup();
                    if (ImGui::Button("OK")) {
                        ImGui::CloseCurrentPopup();
                        ImGui::EndPopup();
                    }
                }
                else {
                    model = model.load3DModel(modelPathTmp, shader, &menu);
                }
            }
            //WireFrame display
            if (ImGui::Checkbox("WireFrame", &menu.isWireFrame())) {
                if (!&menu.isWireFrame())
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }
            else {
                if (&menu.isWireFrame())
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }

            //Background
            ImGui::ColorEdit4("Background", backGroundColorTmp, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_DisplayRGB |
                ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_InputRGB);
            //Material
            ImGui::Text("Material");
            ImGui::ColorEdit4("Ambient Material", ambientMaterialColorTmp, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_DisplayRGB |
                ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_InputRGB);
            ImGui::ColorEdit4("Diffuse Material", diffuseMaterialColorTmp, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_DisplayRGB |
                ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_InputRGB);
            ImGui::ColorEdit4("Specular Material", specularMaterialColorTmp, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_DisplayRGB |
                ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_InputRGB);

            //Shininess
            ImGui::SliderFloat("Shininess", &menu.getShininess(), 0.0f, 100.0f);
            //Scene
            ImGui::Text("Scene");
            ImGui::ColorEdit4("Ambient Scene", ambientLightingColorTmp, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_DisplayRGB |
                ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_InputRGB);
            ImGui::ColorEdit4("Diffuse Scene", diffuseLightingColorTmp, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_DisplayRGB |
                ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_InputRGB);
            ImGui::ColorEdit4("Specular Scene", specularLightingColorTmp, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_DisplayRGB |
                ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_InputRGB);

            //Input
            ImGui::SliderFloat("Mouse Sensitivity", mouse_sensitivity, 0.0f, 1.0f);
            ImGui::SliderFloat("Zoom Sensitivity", zoom_sensitivity, 0.0f, 1.0f);
            ImGui::SliderFloat("Fov Sensitivity", fov_sensitivity, 0.0f, 1.0f);
            camera.setSensitivities(menu.getMouseSensitivity(), menu.getZoomSensitivity(), menu.getFovSensitivity());

            //End menu
            ImGui::End();
            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            //glClearColor(menu.backgroundColor.x, menu.backgroundColor.y, menu.backgroundColor.z, 1.0f);
            glClearColor(backGroundColorTmp[0], backGroundColorTmp[1], backGroundColorTmp[2], 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            //shader.setFloat("distance", explode_distance);
            shader.setMat4("view", camera.getViewMatrix());
            shader.setMat4("projection", camera.getProjectionMatrix());
            shader.setVec3("light.position", camera.getPosition());
            shader.setVec3("view_pos", camera.getPosition());

            //Draw Model
            model.Draw(shader);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            //Reload shader values
            shader.reloadValues(&menu);
            glfwSwapBuffers(window);
            glfwPollEvents();

            process_keypresses(window, deltaTime);
        }
        glDeleteProgram(shader.get_id());
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwTerminate();
    }
    catch (...) {
        std::cerr << "Caught an exception!" << std::endl;
    }
    return 0;
}


void process_keypresses(GLFWwindow* window, float delta_time)
{
    //Arrow Keys Camera Movement
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        camera.orbit(cameraSpeed, 0.f);
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        camera.orbit(-cameraSpeed, 0.f);
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        camera.orbit(0.f, cameraSpeed);
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        camera.orbit(0.f, -cameraSpeed);
    }

    //Fov controls
    if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS)
    {
        camera.narrowFov();
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS)
    {
        camera.widenFov();
    }
}

void mouse_callback(GLFWwindow* window, double x_pos, double y_pos)
{
    if (first_mouse)
    {
        last_x = x_pos;
        last_y = y_pos;
        first_mouse = false;
    }

    float x_offset = x_pos - last_x;
    float y_offset = last_y - y_pos; // reversed since y-coordinates go from bottom to top

    last_x = x_pos;
    last_y = y_pos;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        camera.orbit(x_offset, y_offset);
    }
}

void scroll_callback(GLFWwindow* window, double x_offset, double y_offset)
{
    camera.zoom(y_offset);
}

inline bool file_exists(const std::string& name) {
    if (FILE* file = fopen(name.c_str(), "r")) {
        fclose(file);
        return true;
    }
    else {
        return false;
    }
}