#version 330 core

uniform mat4 modelMtx;
layout (std140) uniform ViewProjectionMtx {
    uniform mat4 viewMtx;
    uniform mat4 projectionMtx;
};

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;

out vec4 gNormal;

void main() {
    gNormal = projectionMtx * vec4(mat3(transpose(inverse(viewMtx * modelMtx))) * vNormal, 0.0);    // Normal in clip space.
    
    gl_Position = projectionMtx * viewMtx * modelMtx * vec4(vPosition, 1.0);
}
