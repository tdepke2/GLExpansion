#version 330 core

uniform sampler2D image;

in vec2 fTexCoords;

out float fragColor;

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(image, 0));
    float result = 0.0;
    for (int x = -2; x < 2; ++x) {
        for (int y = -2; y < 2; ++y) {
            result += texture(image, fTexCoords + vec2(x, y) * texelSize).r;
        }
    }
    
    fragColor = result / (4.0 * 4.0);
}
