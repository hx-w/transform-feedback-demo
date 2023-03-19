#pragma once
#include <string>
#include <vector>
#include <set>
#include <GLFW/glfw3.h>
#include "geometry.h"
#include "shader.h"
#include "viewer.h"
#include "flatten.h"

namespace TFDEMO {

class Render {
public:
    Render();
    ~Render();

    void set_mesh(Vertices&, Faces&, std::shared_ptr<FlattenParam>);

    void init(int, int, const std::string&);
    int draw();
    void cleanup();

    void active_flatten();
    void pause_flatten();

    void transform_feedback_process();

private:
    GLFWwindow* window;
    Shader shader;

    GLuint vao = 0;
    GLuint vbo = 0, ebo = 0;
    GLuint tbos[2], tfo = 0;

    Vertices vertices;
    Faces faces;
    std::shared_ptr<FlattenParam> param;
    int flatten_stage = 0; // 0 init; 1 boundary mapped; 2 animate

    std::vector<std::set<int>> vts_adj;

    Viewer viewer;
};

}