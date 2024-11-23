#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <inih/INIReader.h>
#include "camera.h"
#include "menu.h"
#include "model.h"
#include "mesh.h"
#include <GL/glut.h>
#include <stdio.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>


#if defined(IMGUI_IMPL_OPENGL_ES2)
#endif
#define GL_SILENCE_DEPRECATION

GLFWwindow* window;

const unsigned int SCR_WIDTH = 1080;
const unsigned int SCR_HEIGHT = 720;

// Global values
float last_x, last_y;
bool first_mouse = true;

// Camera
Camera camera;
Menu menu(camera);


//Controls
void process_keypresses(GLFWwindow* window, float deltaTime);
void mouse_callback(GLFWwindow* window, double x_pos, double y_pos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

int main(int* argc, char** argv)
{
    INIReader config("config.ini");
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
    if (config.GetBoolean("Screen", "Fullscreen", true))
    {
        monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        window_width = mode->width;
        window_height = mode->height;
    }
    else
    {
        monitor = NULL;
        window_width = config.GetInteger("Screen", "ScreenWidth", 800);
        window_height = config.GetInteger("Screen", "ScreenHeight", 640);
    }
    GLFWwindow* window = glfwCreateWindow(window_width, window_height, "STL Model Viewer", NULL, NULL);
    camera.setScreenDimensions(window_width, window_height);

    // Make the window's context current
    glfwMakeContextCurrent(window);
    // Set input callbacks
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    // Set vsync
    glfwSwapInterval(1);

    //Menu
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

    //ImGui_ImplOpenGL3_Init(glsl_version);

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
    if (config.GetBoolean("Graphics", "Wireframe", false))
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    //Menu
    
    // Set up input sensitivities
    float mouse_sensitivity = config.GetFloat("Input", "MouseSensitivity", 0.3);
    float zoom_sensitivity = config.GetFloat("Input", "ZoomSensitivity", 0.5);
    float fov_sensitivity = config.GetFloat("Input", "FovSensitivity", 0.4);
    camera.setSensitivities(mouse_sensitivity, zoom_sensitivity, fov_sensitivity);


    // Instantiate, compile and link shader
    Shader shader(
        "shaders/vertex.glsl",
        "shaders/fragment.glsl",
        "shaders/geometry.glsl"
    );
    shader.use(); // There is only have one shader so this one can remain attached

    // Instantiate and load model
    
    Model ourModel(config.Get("Model", "Path", ""));
    // Build model matrix
    glm::mat4 model = glm::mat4(1.0f);
    
    //Model Transformations
    model = glm::translate(model, menu.translate);
    model = glm::scale(model, glm::vec3(menu.scale));
    model = glm::rotate(model, glm::radians(menu.rotationAngle), glm::vec3(menu.rotate));
    // Send model matrix to vertex shader as it remains constant
    shader.setMat4("model", model);

    shader.setFloat("material.shininess", menu.shininess);

    //Material Colors
    shader.setVec3("material.ambient", menu.ambientMaterialColor);
    shader.setVec3("material.diffuse", menu.diffuseMaterialColor);
    shader.setVec3("material.specular", menu.specularMaterialColor);

    //Scene lighting
    shader.setVec3("light.ambient", menu.ambientLightingColor);
    shader.setVec3("light.diffuse", menu.diffuseLightingColor);
    shader.setVec3("light.specular", menu.specularLightingColor);

    //Time and Frame Animation
    float deltaTime, currentFrame, lastFrame = 0;
    bool show_demo_window = true;
    bool show_another_window = false;

    //BackGround and Scene Temporary Values
    float*  backGroundColorTmp = glm::value_ptr(menu.backgroundColor);

    //Material Temporary Values
    float* ambientMaterialColorTmp = glm::value_ptr(menu.ambientMaterialColor);
    float* diffuseMaterialColorTmp = glm::value_ptr(menu.diffuseMaterialColor);
    float* specularMaterialColorTmp = glm::value_ptr(menu.specularMaterialColor);

    //Scene Lighting Temporary Values
    float* ambientLightingColorTmp = glm::value_ptr(menu.ambientLightingColor);
    float* diffuseLightingColorTmp = glm::value_ptr(menu.diffuseLightingColor);
    float* specularLightingColorTmp = glm::value_ptr(menu.specularLightingColor);

    //Model Temporary Values
    //float* modelColorTmp = glm::value_ptr(menu.backgroundColor);;
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
        
        //WireFrame
        ImGui::Checkbox("WireFrame", &menu.wireFrame);
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
        ImGui::SliderFloat("Shininess", &menu.shininess, 0.0f, 100.0f);
        //Scene
        ImGui::Text("Scene");
        ImGui::ColorEdit4("Ambient Scene", ambientLightingColorTmp, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_DisplayRGB |
            ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_InputRGB);
        ImGui::ColorEdit4("Diffuse Scene", diffuseLightingColorTmp, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_DisplayRGB |
            ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_InputRGB);
        ImGui::ColorEdit4("Specular Scene", specularLightingColorTmp, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_DisplayRGB |
            ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_InputRGB);

        //Input
        ImGui::SliderFloat("Mouse Sensitivity", &menu.mouseSensitivity, 0.0f, 1.0f);
        ImGui::SliderFloat("Zoom Sensitivity", &menu.zoomSensitivity, 0.0f, 1.0f);
        ImGui::SliderFloat("Fov Sensitivity", &menu.fovSensitivity, 0.0f, 1.0f);
        
        //End menu
        ImGui::End();
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(menu.backgroundColor.x, menu.backgroundColor.y, menu.backgroundColor.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        //shader.setFloat("distance", explode_distance);
        shader.setMat4("view", camera.getViewMatrix());
        shader.setMat4("projection", camera.getProjectionMatrix());
        shader.setVec3("light.position", camera.getPosition());
        shader.setVec3("view_pos", camera.getPosition());

        // The magic line of code 
        ourModel.Draw(shader);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        glfwPollEvents();

        //process_keypresses(window, deltaTime);
    }
    
    glDeleteProgram(shader.get_id());
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}


void process_keypresses(GLFWwindow* window, float delta_time)
{
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

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

