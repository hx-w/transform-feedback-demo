#include <memory>
#include <iostream>

#include "render.h"
#include "geometry.h"
#include "flatten.h"

int main(int argc, char** argv) {
    auto render = std::make_unique<TFDEMO::Render>();

    TFDEMO::Vertices vertices;
    TFDEMO::Faces faces;

    // load and preprocess
    TFDEMO::load_obj(ASSETS_PATH "/models/mesh.obj", vertices, faces);
    TFDEMO::Edges edges_bound, edges_inner;
    TFDEMO::Flatten::mark_edges(vertices, faces, edges_bound, edges_inner);
    float bound_length = 0;
    TFDEMO::Flatten::reorder_boundary(vertices, faces, edges_bound, bound_length);
    std::clog << "bound_length: " << bound_length << std::endl;

    std::vector<glm::vec3> mapped_boundary;
    TFDEMO::Flatten::map_boundary(vertices, faces, edges_bound, mapped_boundary, bound_length);

    TFDEMO::Weights weights;
    TFDEMO::Flatten::init_weights(vertices, faces, edges_bound, edges_inner, weights);

    std::vector<int> vts_bound, vts_inner;
    TFDEMO::Flatten::transfer_edges_to_vertex_indices(
        std::move(edges_bound), std::move(edges_inner), vts_bound, vts_inner
    );

    // test
    for (int i = 0; i < vts_bound.size(); ++i) {
        vertices[vts_bound[i]].position = mapped_boundary[i];
    }

    render->set_mesh(vertices, faces);

    return render->draw();
}
