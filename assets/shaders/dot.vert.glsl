#version 330 core

layout (location = 0) in vec3 aPos;

out mat4 mvp;

uniform mat4 view;
uniform mat4 projection;

void main() {
    mvp = projection * view;
    gl_Position = vec4(aPos, 1.0f);
//    gl_Position = projection * view * vec4(aPos, 1.0f);
}
