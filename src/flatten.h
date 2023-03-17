#pragma once

#include <vector>
#include <unordered_map>
#include <utility>
#include <glm/glm.hpp>
#include "geometry.h"

namespace TFDEMO {

using OrderedEdge = std::pair<int, int>;
using Edges = std::vector<OrderedEdge>;

struct pair_hash {
    template <class T1, class T2>
    std::size_t operator() (const std::pair<T1, T2> &p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        return h1 ^ h2;  
    }
};

using Weights = std::unordered_map<OrderedEdge, float, pair_hash>;

class Flatten {
public:
    static void mark_edges(
        const Vertices&, const Faces&,
        Edges& edge_bound, Edges& edge_inner
    );

    static void reorder_boundary(
        const Vertices&, const Faces&, Edges&, float&
    );

    static void map_boundary(
        const Vertices&, const Faces&, Edges&, std::vector<glm::vec3>&,
        float, float = 2.0f
    );

    static void init_weights(
        const Vertices&, const Faces&,
        const Edges&, const Edges&,
        Weights& 
    );

    static void transfer_edges_to_vertex_indices(
        Edges&&, Edges&&, std::vector<int>&, std::vector<int>&
    );
};

}