#version 330 core

uniform sampler2D texGlyph;
uniform vec3 color;

in vec2 fTexCoords;

out vec4 fragColor;

void main() {
    //fragColor = vec4(color, texture(texGlyph, fTexCoords).r);
    
    fragColor = vec4(texture(texGlyph, fTexCoords).r, 0.2, 0.2, 1.0);
}
