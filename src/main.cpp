#include <memory>

#include "render.h"
#include "geometry.h"

int main(int argc, char** argv) {
    auto render = std::make_unique<TFDEMO::Render>();

    TFDEMO::Vertices vertices;
    TFDEMO::Faces faces;

    TFDEMO::load_obj(ASSETS_PATH "/models/mesh.obj", vertices, faces);

    render->set_mesh(vertices, faces);

    return render->draw();
}
