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
out vec2 fTexCoords;
out mat3 fTBNMtx;

void main() {
    fPosition = vec3(modelMtx * vec4(vPosition, 1.0));    // Fragment position in world space.
    fTexCoords = vTexCoords;
    mat3 normalMtx = transpose(inverse(mat3(modelMtx)));
    fTBNMtx = mat3(normalize(normalMtx * vTangent), normalize(normalMtx * vBitangent), normalize(normalMtx * vNormal));
    
    gl_Position = projectionMtx * viewMtx * modelMtx * vec4(vPosition, 1.0);
}
