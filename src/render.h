#pragma once
#include <string>
#include <vector>
#include <GLFW/glfw3.h>
#include "geometry.h"
#include "shader.h"
#include "viewer.h"

namespace TFDEMO {

class Render {
public:
    Render();
    ~Render();

    void set_mesh(Vertices&, Faces&);

    void init(int, int, const std::string&);
    int draw();
    void cleanup();

private:
    GLFWwindow* window;
    Shader shader;

    GLuint vao = 0;

    Vertices vertices;
    Faces faces;

    Viewer viewer;
};

}