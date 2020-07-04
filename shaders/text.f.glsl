#version 330 core

uniform sampler2D texFont;
uniform vec3 color;

in vec2 fTexCoords;

out vec4 fragColor;

void main() {
    fragColor = vec4(color, texture(texFont, fTexCoords).r);
    
    //fragColor = vec4(texture(texFont, fTexCoords).r, 0.2, 0.2, 1.0);
}
