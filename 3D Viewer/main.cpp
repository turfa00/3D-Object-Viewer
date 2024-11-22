#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <inih/INIReader.h>
#include "camera.h"
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

//Move elsewhere
// Globals
float last_x, last_y;
bool first_mouse = true;

// Camera
Camera camera;


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
    float tests = config.GetFloat("Model", "TranslateY", 0.0);
    glm::vec3 model_translate_vec = glm::vec3(
        config.GetFloat("Model", "TranslateX", 0.0),
        config.GetFloat("Model", "TranslateY", 0.0),
        config.GetFloat("Model", "TranslateZ", 0.0)
    );
    glm::vec3 model_scale = glm::vec3(
        config.GetFloat("Model", "ScaleXYZ", 1.0)
    );
    float model_rotate_angle = glm::radians(
        config.GetFloat("Model", "RotateAngle", 0.0)
    );
    glm::vec3 model_rotate_axis = glm::vec3(
        config.GetFloat("Model", "RotateAxisX", 0.0),
        config.GetFloat("Model", "RotateAxisY", 0.0),
        config.GetFloat("Model", "RotateAxisZ", 0.0)
    );
    model = glm::translate(model, model_translate_vec);
    model = glm::scale(model, model_scale);
    model = glm::rotate(model, model_rotate_angle, model_rotate_axis);
    // Send model matrix to vertex shader as it remains constant
    shader.setMat4("model", model);

    // Set fragment shader uniforms
    // Material lighting properties
    glm::vec3 material_ambient = glm::vec3(
        config.GetFloat("Material", "AmbientColorR", 1.0),
        config.GetFloat("Material", "AmbientColorG", 1.0),
        config.GetFloat("Material", "AmbientColorB", 1.0)
    );
    shader.setVec3("material.ambient", material_ambient);
    glm::vec3 material_diffuse = glm::vec3(
        config.GetFloat("Material", "DiffuseColorR", 1.0),
        config.GetFloat("Material", "DiffuseColorG", 1.0),
        config.GetFloat("Material", "DiffuseColorB", 1.0)
    );
    shader.setVec3("material.diffuse", material_diffuse);
    glm::vec3 material_specular = glm::vec3(
        config.GetFloat("Material", "SpecularColorR", 1.0),
        config.GetFloat("Material", "SpecularColorG", 1.0),
        config.GetFloat("Material", "SpecularColorB", 1.0)
    );
    shader.setVec3("material.specular", material_specular);
    float material_shininess = config.GetFloat("Material", "Shininess", 32.0);
    shader.setFloat("material.shininess", material_shininess);

    // Light lighting properties
    glm::vec3 light_ambient = glm::vec3(
        config.GetFloat("Light", "AmbientColorR", 1.0),
        config.GetFloat("Light", "AmbientColorG", 1.0),
        config.GetFloat("Light", "AmbientColorB", 1.0));
    shader.setVec3("light.ambient", light_ambient);
    glm::vec3 light_diffuse = glm::vec3(
        config.GetFloat("Light", "DiffuseColorR", 1.0),
        config.GetFloat("Light", "DiffuseColorG", 1.0),
        config.GetFloat("Light", "DiffuseColorB", 1.0));
    shader.setVec3("light.diffuse", light_diffuse);
    glm::vec3 light_specular = glm::vec3(
        config.GetFloat("Light", "SpecularColorR", 1.0),
        config.GetFloat("Light", "SpecularColorG", 1.0),
        config.GetFloat("Light", "SpecularColorB", 1.0));
    shader.setVec3("light.specular", light_specular);

    // Background color
    glm::vec3 background_color = glm::vec3(
        config.GetFloat("Graphics", "BackgroundColorR", 0.8),
        config.GetFloat("Graphics", "BackgroundColorG", 0.0),
        config.GetFloat("Graphics", "BackgroundColorB", 0.0)
    );

    float explode_distance = 2.0;
    bool should_explode = config.GetBoolean("Graphics", "ExplodeOnStart", true);
    float deltaTime,
        currentFrame, lastFrame = 0;
    bool show_demo_window = true;
    bool show_another_window = false;
    while (!glfwWindowShouldClose(window))
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        // Delta time calculations
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &show_another_window);
        
        
            
        ImGui::End();
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(background_color.r, background_color.g,
            background_color.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Vertex shader uniforms
        if (explode_distance > 0 && should_explode)
        {
            explode_distance -= 0.01;
        }
        else
        {
            explode_distance = 0;
        }
        shader.setFloat("distance", explode_distance);
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

