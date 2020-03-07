#version 330 core

uniform vec3 lightColor;

in vec3 fNormal;
in vec2 fTexCoord;

out vec4 FragColor;

void main() {
    FragColor = vec4(lightColor, 1.0);
}
