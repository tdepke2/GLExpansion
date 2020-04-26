#version 330 core

uniform mat4 modelMtx;
uniform mat4 viewMtx;
uniform mat4 projectionMtx;

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;

out vec3 fPosition;
out vec3 fNormal;

void main() {
    fPosition = vec3(modelMtx * vec4(vPosition, 1.0));    // Fragment position in world space.
    fNormal = mat3(transpose(inverse(modelMtx))) * vNormal;    // Normal in world space.
    
    gl_Position = projectionMtx * viewMtx * vec4(fPosition, 1.0);
}
