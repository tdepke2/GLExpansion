#version 330 core

const float LENGTH = 0.4;

layout (triangles) in;
in vec4 gNormal[];

layout (line_strip, max_vertices = 6) out;

void generateLine(int index) {
    gl_Position = gl_in[index].gl_Position;
    EmitVertex();
    gl_Position = gl_in[index].gl_Position + gNormal[index] * LENGTH;
    EmitVertex();
    EndPrimitive();
}

void main() {
    generateLine(0);
    generateLine(1);
    generateLine(2);
}
