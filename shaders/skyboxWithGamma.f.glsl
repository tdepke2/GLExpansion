#version 330 core

const float GAMMA = 2.2;

uniform samplerCube skybox;

in vec3 fTexCoords;

out vec4 fragColor;

void main() {
    fragColor = vec4(pow(texture(skybox, fTexCoords).rgb, vec3(1.0 / GAMMA)), 1.0);    // Apply gamma correction.
}
