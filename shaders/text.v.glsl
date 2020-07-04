#version 330 core

uniform mat4 projectionMtx;

layout (location = 0) in vec4 vPosTex;

out vec2 fTexCoords;

void main() {
    gl_Position = projectionMtx * vec4(vPosTex.xy, 0.0, 1.0);
    fTexCoords = vPosTex.zw;
}
