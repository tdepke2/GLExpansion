#version 330 core

const float GAMMA = 2.2;

uniform samplerCube skybox;

in vec3 fTexCoords;

out vec4 fragColor;

void main() {
    vec3 color = texture(skybox, fTexCoords).rgb;
    
    vec3 mappedColor = color / (color + vec3(1.0));    // Reinhard tone mapping.
    fragColor = vec4(pow(mappedColor, vec3(1.0 / GAMMA)), 1.0);    // Apply gamma correction.
}
