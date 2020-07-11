#version 330 core

const uint MAX_NUM_BONES = 128u;

uniform mat4 modelMtx;
layout (std140) uniform ViewProjectionMtx {
    uniform mat4 viewMtx;
    uniform mat4 projectionMtx;
};
uniform mat4 boneTransforms[MAX_NUM_BONES];

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoords;
layout (location = 3) in vec3 vTangent;
layout (location = 4) in vec3 vBitangent;
layout (location = 5) in uvec4 vBone;
layout (location = 6) in vec4 vWeight;

out vec3 fPosition;
out mat3 fTBNMtx;
out vec2 fTexCoords;

void main() {
    mat4 boneMtx = boneTransforms[vBone[0]] * vWeight[0];
    boneMtx +=     boneTransforms[vBone[1]] * vWeight[1];
    boneMtx +=     boneTransforms[vBone[2]] * vWeight[2];
    boneMtx +=     boneTransforms[vBone[3]] * vWeight[3];
    
    fPosition = vec3(viewMtx * modelMtx * boneMtx * vec4(vPosition, 1.0));    // Fragment position in view space.
    mat3 normalMtx = transpose(inverse(mat3(viewMtx * modelMtx * boneMtx)));
    fTBNMtx = mat3(normalize(normalMtx * vTangent), normalize(normalMtx * vBitangent), normalize(normalMtx * vNormal));    // Need to put the normal into view space too.
    fTexCoords = vTexCoords;
    
    gl_Position = projectionMtx * vec4(fPosition, 1.0);
}
