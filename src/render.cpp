#include <iostream>
#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "render.h"

namespace TFDEMO {

Render::Render(): viewer(this) {
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
    glEnable(GL_CULL_FACE);
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

void Render::set_mesh(Vertices& vs, Faces& fs, std::shared_ptr<FlattenParam> prm) {
    vertices.swap(vs);
    faces.swap(fs);
    // transfer prm to param
    param = prm;
    // create a vertex array object
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // create a vertex buffer object
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
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Aii));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bi));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, prod));
    glEnableVertexAttribArray(3);

    // unbind the VAO
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    vts_adj.clear();
    vts_adj.resize(vertices.size());
    for (int i = 0; i < faces.size(); ++i) {
        for (int j = 0; j < 3; ++j) {
            vts_adj[faces[i].indices[j]].insert(faces[i].indices[(j + 1) % 3]);
            vts_adj[faces[i].indices[j]].insert(faces[i].indices[(j + 2) % 3]);
        }
    }

    // create transform feedback buffer object
    glGenBuffers(2, tbos); // 第一个tbo存newPos, 第二个tbo存其他的
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, tbos[0]);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, vertices.size() * sizeof(glm::vec3), NULL, GL_DYNAMIC_READ);
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, tbos[1]);
    glBufferData(
        GL_TRANSFORM_FEEDBACK_BUFFER,
        // vertices.size() * (sizeof(glm::vec3) * 2+ sizeof(float)),
        vertices.size() * (sizeof(float)),
        NULL, GL_DYNAMIC_READ
    );
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);

    // create transform feedback object
    glGenTransformFeedbacks(1, &tfo);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, tfo);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tbos[0]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, tbos[1]);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);
}

void Render::active_flatten() {
    if (flatten_stage == 0) {
        flatten_stage = 1;

        // map boundary
        for (auto i = 0; i < param->vts_bound.size(); ++i) {
            auto& v = vertices[param->vts_bound[i]];
            v.position = param->mapped_boundary[i];
            v.Aii = 0;
        }

        // compute Bi, save Aii
        for (auto r = 0; r < param->vts_inner.size(); ++r) {
            auto vi = param->vts_inner[r];
            auto& v = vertices[vi];
            v.Bi = glm::vec3(0.0);
            // for (auto c = 0; c < param->vts_bound.size(); ++c) {
            //     auto vj = param->vts_bound[c];
            //     v.Bi -= get_val(param->weights, vi, vj) * param->mapped_boundary[c];
            // }
            v.Aii = get_val(param->weights, vi, vi);
        }
    }
    else if (flatten_stage == 1) {
        flatten_stage = 2;
        Flatten::update_prod(vertices, param.get(), vts_adj, nullptr);
    }
    std::clog << "flatten stage: " << flatten_stage << std::endl;
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Render::pause_flatten() {
    if (flatten_stage == 2) {
        flatten_stage = 1;
    }
}

int Render::draw() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // set background
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        viewer.processInput(window);
        glLineWidth(2.0f);
        if (vao != 0) {
            shader.use();
            glBindVertexArray(vao);
            // draw triangles
            glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, tfo);
            glBeginTransformFeedback(GL_TRIANGLES);
            glDrawElements(GL_TRIANGLES, faces.size() * 3, GL_UNSIGNED_INT, 0);
            glEndTransformFeedback();

            transform_feedback_process();
        }

        auto currentFrame = static_cast<float>(glfwGetTime());
		viewer.deltaTime = currentFrame - viewer.lastFrame;
		viewer.lastFrame = currentFrame;
        auto projection = glm::perspective(
            glm::radians(viewer.fov),
			float(viewer.width) /(viewer.height),
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

void Render::transform_feedback_process() {
    if (flatten_stage < 2) return;
    // read tbos[0]
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, tbos[0]);

    /// 读取缓冲区数据 - glMapBufferRange
    // void* rawData = glMapBufferRange(
    //     GL_TRANSFORM_FEEDBACK_BUFFER,
    //     0,
    //     vertices.size() * sizeof(glm::vec3),
    //     GL_MAP_READ_BIT
    // );
    // glm::vec3* feedback = (glm::vec3*)rawData;
    // glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);

    // 读取缓冲区数据 - glGetBufferSubData
    glm::vec3* feedback = new glm::vec3[vertices.size()];
    glGetBufferSubData(
        GL_TRANSFORM_FEEDBACK_BUFFER,
        0,
        vertices.size() * sizeof(glm::vec3),
        feedback
    );

    // read tbos[1]
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, tbos[1]);
    float* feedback2 = new float[vertices.size()];
    glGetBufferSubData(
        GL_TRANSFORM_FEEDBACK_BUFFER,
        0,
        vertices.size() * sizeof(float),
        feedback2
    );

    float s1 = 0, s2 = 0;
    for (auto i = 0; i < vertices.size(); ++i) {
        // if (feedback2[i] != vertices[i].Aii)
            // std::clog << feedback2[i] << " " << vertices[i].Aii << std::endl;
        s1 += feedback2[i];
        s2 += vertices[i].Aii;
    }
    std::clog << s1 << " " << s2 << " " << vertices.size() << std::endl;

    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);

    Flatten::update_prod(vertices, param.get(), vts_adj, feedback);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    delete[] feedback;
    delete[] feedback2;
}

}