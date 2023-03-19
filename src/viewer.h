#pragma once

#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>

struct GLFWwindow;

namespace TFDEMO {
class Render;
class Viewer {
public:
    Viewer(Render* render): render(render) {};

    void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    void mouse_callback(GLFWwindow* window, float xpos, float ypos);
    void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    void scroll_callback(GLFWwindow* window, float xoffset, float yoffset);
    void processInput(GLFWwindow* window);

    // camera
    glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f, 10.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);
    float cameraSpeed = 8.f;

    bool leftMousePressed = false;
    bool firstMouse = true;
    float yaw   = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
    float pitch =  0.0f;
    float lastX =  800.0f / 2.0;
    float lastY =  600.0 / 2.0;
    float fov   =  45.0f;

    float deltaTime = 0.0f;	// time between current frame and last frame
    float lastFrame = 0.0f;

    bool all_visible = true;
    glm::vec4 bgColor = glm::vec4(0.1f, 0.1f, 0.1f, 1.00f);

    float realX = 0.0f;
    float realY = 0.0f;

    // light
    glm::vec3 lightPos = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);

    int width = 800;
    int height = 600;

    Render* render;

    bool key_T_pressed = false;
    bool key_P_pressed = false;
};

}