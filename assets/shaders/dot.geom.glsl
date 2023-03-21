#version 330 core
layout (points) in;
layout (line_strip, max_vertices = 36) out;

in mat4 mvp[];
out vec4 color;

void main() {
    int sectors = 32;
    float radius = 1.0f;
    float PI = 3.1415926;

    vec4 center = gl_in[0].gl_Position;
    vec4 verts[36];

    // generate a discrete circle
    for (int i = 0; i < sectors; i++) {
        float u = (1.0 * i) / sectors;
        float theta = 2 * PI * u;

         verts[i] = center + vec4(
            radius * cos(theta),
            radius * sin(theta),
            0.0,
            1.0
         );
         verts[i] = mvp[0] * verts[i];
    }

    for (int i = 0; i < sectors + 1; i++) {
        gl_Position = verts[i % sectors];
        color = vec4(1.0*i / sectors, 1.0, 0.0, 1.0);
        EmitVertex();
    }
    EndPrimitive();
}
