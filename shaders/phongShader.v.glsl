#version 330 core

uniform mat4 modelMtx;
layout (std140) uniform ViewProjectionMtx {
    uniform mat4 viewMtx;
    uniform mat4 projectionMtx;
};
uniform mat4 lightSpaceMtx;

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoords;

out vec3 fPosition;
out vec3 fNormal;
out vec2 fTexCoords;
out vec4 fPositionLightSpace;

void main() {
    fPosition = vec3(viewMtx * modelMtx * vec4(vPosition, 1.0));    // Fragment position in view space.
    fNormal = mat3(transpose(inverse(viewMtx * modelMtx))) * vNormal;    // Need to put the normal into view space too.
    fTexCoords = vTexCoords;
    fPositionLightSpace = lightSpaceMtx * modelMtx * vec4(vPosition, 1.0);
    
    gl_Position = projectionMtx * vec4(fPosition, 1.0);
}
