#version 330 core

layout (std140) uniform ViewProjectionMtx {
    uniform mat4 viewMtx;
    uniform mat4 projectionMtx;
};

layout (location = 0) in vec3 vPosition;
layout (location = 5) in vec3 vTranslation;
layout (location = 6) in vec3 vColor;
layout (location = 7) in vec3 vPhongVals;
layout (location = 8) in vec3 vAttenuation;

out vec3 fColor;

void main() {
    fColor = vColor;
    gl_Position = projectionMtx * viewMtx * vec4(vPosition + vTranslation, 1.0);
}
