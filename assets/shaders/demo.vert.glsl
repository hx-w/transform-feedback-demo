#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in int seq_ind;
// layout (location = 1) in vec3 aColor;
// layout (location = 2) in vec3 aNorm;

out vec3 objectColor;

uniform mat4 view;
uniform mat4 projection;

void main() {
    objectColor = vec3(0.0, 0.0, 0.0);
    gl_Position = projection * view * vec4(aPos, 1.0f);
}

