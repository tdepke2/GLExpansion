#version 330 core

uniform mat4 modelMtx;
layout (std140) uniform ViewProjectionMtx {
    uniform mat4 viewMtx;
    uniform mat4 projectionMtx;
};

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoords;
layout (location = 3) in vec3 vTangent;
layout (location = 4) in vec3 vBitangent;

out vec3 fPosition;
out mat3 fTBNMtx;
out vec2 fTexCoords;

void main() {
    fPosition = vec3(viewMtx * modelMtx * vec4(vPosition, 1.0));    // Fragment position in view space.
    mat3 normalMtx = transpose(inverse(mat3(viewMtx * modelMtx)));
    fTBNMtx = mat3(normalize(normalMtx * vTangent), normalize(normalMtx * vBitangent), normalize(normalMtx * vNormal));    // Need to put the normal into view space too.
    fTexCoords = vTexCoords;
    
    gl_Position = projectionMtx * vec4(fPosition, 1.0);
}
