#version 330 core

uniform samplerCube skybox;

in vec3 fTexCoords;

out vec4 fragColor;

void main() {
    fragColor = texture(skybox, fTexCoords);
}
