#version 330 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vColor;
layout (location = 2) in vec2 vTexCoord;

out vec3 fColor;
out vec2 fTexCoord;

void main() {
    gl_Position = projection * view * model * vec4(vPos, 1.0);
    fColor = vColor;
    fTexCoord = vTexCoord;
}
