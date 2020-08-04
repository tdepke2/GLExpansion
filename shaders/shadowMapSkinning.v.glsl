#version 330 core

const uint MAX_NUM_BONES = 128u;

uniform mat4 modelMtx;
uniform mat4 lightSpaceMtx;
uniform mat4 boneTransforms[MAX_NUM_BONES];

layout (location = 0) in vec3 vPosition;
layout (location = 5) in uint vBone;
layout (location = 6) in vec4 vWeight;

void main() {
    mat4 boneMtx = boneTransforms[vBone & 0xFFu] * vWeight[0];
    boneMtx +=     boneTransforms[(vBone >> 8) & 0xFFu] * vWeight[1];
    boneMtx +=     boneTransforms[(vBone >> 16) & 0xFFu] * vWeight[2];
    boneMtx +=     boneTransforms[(vBone >> 24) & 0xFFu] * vWeight[3];
    
    gl_Position = lightSpaceMtx * modelMtx * boneMtx * vec4(vPosition, 1.0);
}
