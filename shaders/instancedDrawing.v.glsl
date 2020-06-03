#version 330 core

layout (std140) uniform ViewProjectionMtx {
    uniform mat4 viewMtx;
    uniform mat4 projectionMtx;
};

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoords;
layout (location = 3) in mat4 instanceMtx;

out vec2 fTexCoords;

void main() {
    fTexCoords = vTexCoords;
    
    gl_Position = projectionMtx * viewMtx * instanceMtx * vec4(vPosition, 1.0);
}
