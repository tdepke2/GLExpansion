#version 330 core

uniform mat4 viewMtx;
uniform mat4 projectionMtx;

layout (location = 0) in vec3 vPosition;

out vec3 fTexCoords;

void main() {
    fTexCoords = vPosition;
    fTexCoords.x = -fTexCoords.x;
    
    vec4 position = projectionMtx * viewMtx * vec4(vPosition, 1.0);
    gl_Position = position.xyww;
}
