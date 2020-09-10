#version 330 core

const float LENGTH = 1.0;

layout (std140) uniform ViewProjectionMtx {
    uniform mat4 viewMtx;
    uniform mat4 projectionMtx;
};

layout (points) in;
in mat4 gModelMtx[1];

layout (line_strip, max_vertices = 6) out;
out vec3 color;

void main() {
    color = vec3(1.0, 0.0, 0.0);
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();
    gl_Position = projectionMtx * viewMtx * gModelMtx[0] * vec4(LENGTH, 0.0, 0.0, 1.0);
    EmitVertex();
    EndPrimitive();
    
    color = vec3(0.0, 1.0, 0.0);
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();
    gl_Position = projectionMtx * viewMtx * gModelMtx[0] * vec4(0.0, LENGTH, 0.0, 1.0);
    EmitVertex();
    EndPrimitive();
    
    color = vec3(0.0, 0.0, 1.0);
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();
    gl_Position = projectionMtx * viewMtx * gModelMtx[0] * vec4(0.0, 0.0, LENGTH, 1.0);
    EmitVertex();
    EndPrimitive();
}
