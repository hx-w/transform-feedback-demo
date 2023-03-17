#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "viewer.h"
#include "render.h"

namespace TFDEMO {
void Viewer::processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    float cameraMove = cameraSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cameraPos += cameraMove * cameraFront;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraMove * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraMove;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraMove;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        cameraPos -= cameraMove * glm::vec3(0.0f, 1.0f, 0.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        cameraPos += cameraMove * glm::vec3(0.0f, 1.0f, 0.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS && !key_T_pressed) {
        render->active_flatten();
        key_T_pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !key_P_pressed) {
        render->pause_flatten();
        key_P_pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE) {
        key_T_pressed = false;
    }
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) {
        key_P_pressed = false;
    }
}

void Viewer::framebuffer_size_callback(
    GLFWwindow* window, int width, int height
) {
    glViewport(0, 0, width, height);
    this->width = width;
    this->height = height;
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void Viewer::mouse_callback(
    GLFWwindow* window, double xposIn, double yposIn
) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    realX = xpos;
    realY = ypos;

    if (!leftMousePressed)
        return;

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;  // change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front(cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
                    sin(glm::radians(pitch)),
                    sin(glm::radians(yaw)) * cos(glm::radians(pitch)));
    cameraFront = glm::normalize(front);
}

void Viewer::scroll_callback(
    GLFWwindow* window, double xoffset, double yoffset
) {
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f;
}

void Viewer::mouse_button_callback(
    GLFWwindow* window, int button, int action, int mods
) {
    if (action == GLFW_PRESS) {
        switch (button) {
            case GLFW_MOUSE_BUTTON_LEFT:
                leftMousePressed = true;
                firstMouse = true;
                break;
            case GLFW_MOUSE_BUTTON_MIDDLE:
                break;
            case GLFW_MOUSE_BUTTON_RIGHT:
                break;
            default:
                return;
        }
    }
    if (action == GLFW_RELEASE) {
        switch (button) {
            case GLFW_MOUSE_BUTTON_LEFT:
                leftMousePressed = false;
                break;
            case GLFW_MOUSE_BUTTON_MIDDLE:
                break;
            case GLFW_MOUSE_BUTTON_RIGHT:
                break;
            default:
                return;
        }
    }
}
}