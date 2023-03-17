#pragma once

#include <string>
#include <vector>

namespace TFDEMO {

struct Vertex {
    float positions[3];
    bool on_boundary = false;
};

struct Face {
    int indices[3];
};

using Vertices = std::vector<Vertex>;
using Faces = std::vector<Face>;

void load_obj(const std::string&, Vertices&, Faces&);

}