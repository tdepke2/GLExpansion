#version 330 core

layout (std140) uniform ViewProjectionMtx {
    uniform mat4 viewMtx;
    uniform mat4 projectionMtx;
};

layout (location = 0) in vec3 vPosition;
layout (location = 5) in mat4 vModelMtx;
layout (location = 9) in vec3 vColor;
layout (location = 10) in vec3 vPhongVals;
layout (location = 11) in vec3 vAttenuation;

out vec3 fLightPositionVS;
out vec3 fColor;
out vec3 fPhongVals;
out vec3 fAttenuation;

void main() {
    fLightPositionVS = vec3(viewMtx * vec4(vModelMtx[3].xyz, 1.0));
    fColor = vColor;
    fPhongVals = vPhongVals;
    fAttenuation = vAttenuation;
    
    gl_Position = projectionMtx * viewMtx * vModelMtx * vec4(vPosition, 1.0);
}
