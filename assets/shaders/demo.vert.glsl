#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 Bi;
layout (location = 2) in vec3 prod;
layout (location = 3) in float Aii;

out vec3 newPos;
out float Aii_out;
out vec3 Bi_out;
out vec3 prod_out;

out vec3 objectColor;
uniform mat4 view;
uniform mat4 projection;

void main() {
    Aii_out = Aii;
    Bi_out = Bi;
    prod_out = prod;

    if (Aii <= 0.0 || length(prod) == 0.0f) {
        newPos = aPos;
    }
    else {
        newPos = (1.0 / Aii) * (Bi - prod);
    }

    gl_Position = projection * view * vec4(newPos, 1.0f);
    objectColor = vec3(1.0, 1.0, 1.0);
}

