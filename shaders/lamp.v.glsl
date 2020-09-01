#version 330 core

layout (std140) uniform ViewProjectionMtx {
    uniform mat4 viewMtx;
    uniform mat4 projectionMtx;
};

layout (location = 0) in vec3 vPosition;
layout (location = 5) in mat4 vModelMtx;
layout (location = 9) in vec3 vColor;

out vec3 fColor;

void main() {
    fColor = vColor;
    gl_Position = projectionMtx * viewMtx * vec4(vPosition + vModelMtx[3].xyz, 1.0);
}
