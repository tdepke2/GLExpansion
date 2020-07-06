#version 330 core

uniform sampler2D tex;
uniform vec4 color;

in vec2 fTexCoords;

out vec4 fragColor;

void main() {
    fragColor = texture(tex, fTexCoords) * color;
}
