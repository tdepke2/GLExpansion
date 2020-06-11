#version 330 core

uniform mat4 modelMtx;
uniform mat4 lightSpaceMtx;

layout (location = 0) in vec3 vPosition;

void main() {
    gl_Position = lightSpaceMtx * modelMtx * vec4(vPosition, 1.0);
}
