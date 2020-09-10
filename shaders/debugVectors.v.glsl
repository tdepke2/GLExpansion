#version 330 core

layout (std140) uniform ViewProjectionMtx {
    uniform mat4 viewMtx;
    uniform mat4 projectionMtx;
};

layout (location = 0) in mat4 vModelMtx;

out mat4 gModelMtx;

void main() {
    gModelMtx = vModelMtx;
    
    gl_Position = projectionMtx * viewMtx * vModelMtx * vec4(0.0, 0.0, 0.0, 1.0);
}
