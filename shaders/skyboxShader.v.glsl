#version 330 core

layout (std140) uniform ViewProjectionMtx {
    uniform mat4 viewMtx;
    uniform mat4 projectionMtx;
};

layout (location = 0) in vec3 vPosition;

out vec3 fTexCoords;

void main() {
    fTexCoords = vPosition;
    fTexCoords.x = -fTexCoords.x;
    
    vec4 position = projectionMtx * vec4(mat3(viewMtx) * vPosition, 1.0);
    gl_Position = position.xyww;
}
