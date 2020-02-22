#version 330 core

uniform sampler2D tex1;
uniform sampler2D tex2;

in vec3 fColor;
in vec2 fTexCoord;

out vec4 FragColor;

void main() {
    FragColor = mix(texture(tex1, fTexCoord), texture(tex2, fTexCoord), 0.2);
}
