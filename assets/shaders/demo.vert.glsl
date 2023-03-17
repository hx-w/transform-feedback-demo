#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in int on_boundary;
// layout (location = 1) in vec3 aColor;
// layout (location = 2) in vec3 aNorm;

out vec3 objectColor;
out vec3 Norm;
out vec3 FragPos;

// uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    objectColor = vec3(0.0, 0.0, 0.0);
    // Norm = aNorm;
    Norm = vec3(0.0, 0.0, 1.0);

    FragPos = vec3(vec4(aPos, 1.0f));
    gl_Position = projection * view * vec4(aPos, 1.0f);
}

