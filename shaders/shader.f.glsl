#version 330 core

uniform sampler2D tex;

in vec3 fColor;
in vec2 fTexCoord;

out vec4 FragColor;

void main() {
    FragColor = texture(tex, fTexCoord) * vec4(fColor, 1.0);
}
