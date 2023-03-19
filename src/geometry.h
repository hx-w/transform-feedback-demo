#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace TFDEMO {

struct Vertex {
    glm::vec3 position;
    float Aii = -1.0f;
    glm::vec3 Bi = glm::vec3(0.0f);
    glm::vec3 prod = glm::vec3(0.0f);
};

struct Face {
    glm::uvec3 indices;

    bool operator==(const Face& other) const {
        return (indices.x == other.indices.x && indices.y == other.indices.y && indices.z == other.indices.z) \
                || (indices.x == other.indices.x && indices.y == other.indices.z && indices.z == other.indices.y) \
                || (indices.x == other.indices.y && indices.y == other.indices.z && indices.z == other.indices.x) \
                || (indices.x == other.indices.y && indices.y == other.indices.x && indices.z == other.indices.z) \
                || (indices.x == other.indices.z && indices.y == other.indices.x && indices.z == other.indices.y) \
                || (indices.x == other.indices.z && indices.y == other.indices.y && indices.z == other.indices.x);
    }
};

using Vertices = std::vector<Vertex>;
using Faces = std::vector<Face>;

void load_obj(const std::string&, Vertices&, Faces&);

}