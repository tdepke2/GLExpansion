#version 330 core

uniform mat4 modelMtx;
layout (std140) uniform ViewProjectionMtx {
    uniform mat4 viewMtx;
    uniform mat4 projectionMtx;
};

layout (location = 0) in vec3 vPosition;

void main() {
    gl_Position = projectionMtx * viewMtx * modelMtx * vec4(vPosition, 1.0);
}
