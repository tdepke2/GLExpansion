#version 330 core

uniform mat4 viewMtx;
uniform mat4 projectionMtx;

layout (location = 0) in vec3 vPosition;

out vec3 fTexCoords;

void main() {
    fTexCoords = vPosition;
    
    gl_Position = (projectionMtx * viewMtx * vec4(vPosition, 1.0)).xyww;
}
