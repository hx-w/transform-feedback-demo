#include <iostream>
#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
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
    dot_shader = Shader(
        ASSETS_PATH "/shaders/dot.vert.glsl",
        ASSETS_PATH "/shaders/dot.frag.glsl",
        ASSETS_PATH "/shaders/dot.geom.glsl"
    );

    glm::vec3 dot_list[] = {
        glm::vec3(-0.0f, 0.0f, 1.0f),
        glm::vec3(-0.0f, 1.0f, 0.0f),
        glm::vec3(-0.0f, 0.0f, -1.0f),
    };
    glGenVertexArrays(1, &dot_vao);
    glBindVertexArray(dot_vao);
    glGenBuffers(1, &dot_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, dot_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(dot_list), dot_list, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
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

void Render::set_mesh(Vertices& vs, Faces& fs, std::shared_ptr<FlattenParam> prm) {
    vertices.swap(vs);
    faces.swap(fs);
    vertices_backup.clear();
    vertices_backup.assign(vertices.begin(), vertices.end());
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
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bi));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, prod));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Aii));
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
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER,
        vertices.size() * sizeof(glm::vec3), NULL, GL_DYNAMIC_READ);
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, tbos[1]);
    glBufferData(
        GL_TRANSFORM_FEEDBACK_BUFFER,
        vertices.size() * (sizeof(glm::vec3) * 2 + sizeof(float)),
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
            v.Aii = -1;
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
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        viewer.processInput(window);
        
        glPointSize(5.0f);
        if (viewer.key_H_pressed){
            // draw dots
            dot_shader.use();
            glBindVertexArray(dot_vao);
            glDrawArrays(GL_POINTS, 0, 3);
            glBindVertexArray(0);
        }

        if (vao != 0 && !viewer.key_H_pressed) {
            shader.use();
            glBindVertexArray(vao);
            glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, tfo);
            // draw triangles
            glBeginTransformFeedback(draw_mode);
            if (draw_mode == GL_POINTS) {
                glDrawArrays(draw_mode, 0, vertices.size());
            }
            else if (draw_mode == GL_TRIANGLES) {
                glDrawElements(draw_mode, faces.size() * 3, GL_UNSIGNED_INT, 0);
            }
            glEndTransformFeedback();
            glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
            glBindVertexArray(0);
            transform_feedback_process();
        }

        auto currentFrame = static_cast<float>(glfwGetTime());
		viewer.deltaTime = currentFrame - viewer.lastFrame;
		viewer.lastFrame = currentFrame;
        auto projection = glm::perspective(
            glm::radians(viewer.fov),
			float(viewer.width) /float(viewer.height),
			0.1f, 500.0f
		);

		auto view = glm::lookAt(
			viewer.cameraPos,
			viewer.cameraPos + viewer.cameraFront,
			viewer.cameraUp
		);

        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        dot_shader.setMat4("projection", projection);
        dot_shader.setMat4("view", view);

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
    struct VaryPack {
        float Aii_out;
        glm::vec3 Bi_out;
        glm::vec3 prod_out;
    };

    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, tbos[1]);
    VaryPack* feedback_check = new VaryPack[vertices.size()];
    glGetBufferSubData(
        GL_TRANSFORM_FEEDBACK_BUFFER,
        0,
        sizeof(VaryPack) * vertices.size(),
        feedback_check
    );

    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);

    Flatten::update_prod(vertices, param.get(), vts_adj, feedback);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    delete[] feedback;
    delete[] feedback_check;
}

void Render::change_draw_mode(GLenum mode) {
    if (mode != GL_POINTS && mode != GL_TRIANGLES) return;

    draw_mode = mode;

    vertices.clear();
    vertices.assign(vertices_backup.begin(), vertices_backup.end());

    flatten_stage = 0;    
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

}
