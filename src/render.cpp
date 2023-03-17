#include <iostream>
#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "render.h"

namespace TFDEMO {

Render::Render() {
    // create a window with the specified width, height and title and initialize
    // OpenGL
    init(800, 600, "Demo");
    shader = Shader(
        ASSETS_PATH "/shaders/demo.vert.glsl",
        ASSETS_PATH "/shaders/demo.frag.glsl"
    );
}

Render::~Render() {
    cleanup();
    exit(EXIT_SUCCESS);
}

void Render::init(int width, int height, const std::string& title) {
    glfwInit();
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif
    glfwWindowHint(GLFW_SAMPLES, 4);
    // glfw window creation
    // --------------------
    window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);
    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }

    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    // glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);
    glfwSwapInterval(1); // Enable vsync

    glfwSetWindowUserPointer(window, &viewer);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
        static_cast<Viewer*>(glfwGetWindowUserPointer(window))->framebuffer_size_callback(window, width, height);
    });
    glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
        static_cast<Viewer*>(glfwGetWindowUserPointer(window))->scroll_callback(window, xoffset, yoffset);
    });
    glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int modes) {
        static_cast<Viewer*>(glfwGetWindowUserPointer(window))->mouse_button_callback(window, button, action, modes);
    });
    glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
        static_cast<Viewer*>(glfwGetWindowUserPointer(window))->mouse_callback(window, xpos, ypos);
    });
}

void Render::cleanup() {
	glDeleteProgram(shader.ID);
	glDeleteVertexArrays(1, &vao);

	glfwDestroyWindow(window);
	glfwTerminate();
}

void Render::set_mesh(Vertices& vs, Faces& fs) {
    vertices.swap(vs);
    faces.swap(fs);

    // create a vertex array object
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // create a vertex buffer object
    GLuint vbo = 0, ebo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);

    // create an element array
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(Face), faces.data(), GL_DYNAMIC_DRAW);

    // specify the layout of the vertex data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 1, GL_INT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, on_boundary));
    glEnableVertexAttribArray(1);

    // unbind the VAO
    glBindVertexArray(0);
}

int Render::draw() {
    // loop until the user presses ESC or the window is closed programatically
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // set background
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        viewer.processInput(window);
        // if vao is not initialized, initialize it
        if (vao != 0) {
            shader.use();
            glBindVertexArray(vao);
            glLineWidth(2.0f);
            // draw the triangle
            glDrawElements(GL_TRIANGLES, faces.size() * 3, GL_UNSIGNED_INT, 0);
        }

        auto currentFrame = static_cast<float>(glfwGetTime());
		viewer.deltaTime = currentFrame - viewer.lastFrame;
		viewer.lastFrame = currentFrame;
        auto projection = glm::perspective(
            glm::radians(viewer.fov),
			(float)viewer.width / (float)viewer.height,
			0.1f, 500.0f
		);

		auto view = glm::lookAt(
			viewer.cameraPos,
			viewer.cameraPos + viewer.cameraFront,
			viewer.cameraUp
		);

        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        shader.setVec3("lightColor",  viewer.lightColor);
        shader.setVec3("lightPos", viewer.lightPos);
        shader.setVec3("viewPos", viewer.cameraPos);
        shader.setBool("ignoreLight", true);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    return EXIT_SUCCESS;
}
}