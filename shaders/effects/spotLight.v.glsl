#version 330 core

uniform mat4 modelMtx;
layout (std140) uniform ViewProjectionMtx {
    uniform mat4 viewMtx;
    uniform mat4 projectionMtx;
};

layout (location = 0) in vec3 vPosition;

out vec3 fLightPositionVS;
out vec3 fLightDirectionVS;

void main() {
    fLightPositionVS = vec3(viewMtx * vec4(modelMtx[3].xyz, 1.0));
    fLightDirectionVS = mat3(viewMtx) * mat3(modelMtx) * vec3(0.0, 0.0, 1.0);    // not sure about this one #################################
    
    gl_Position = projectionMtx * viewMtx * modelMtx * vec4(vPosition, 1.0);
}
