#version 330 core

in vec3 fColor;

out vec4 fragColor;

void main() {
    fragColor = vec4(fColor, 1.0);
}
