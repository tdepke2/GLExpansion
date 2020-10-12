#version 330 core

const float EDGE_BIAS = 0.5;

uniform sampler2D image;

in vec2 fTexCoords;

out float fragColor;

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(image, 0));
    float centerOcclusion = texture(image, fTexCoords).r;
    float result = 0.0;
    float count = 0.0;
    
    for (int x = -2; x < 2; ++x) {
        for (int y = -2; y < 2; ++y) {
            float currentOcclusion = texture(image, fTexCoords + vec2(x, y) * texelSize).r;
            if (currentOcclusion - centerOcclusion < EDGE_BIAS) {    // Reduce the effect of bleeding white to black along edges (causes a halo to appear on edges otherwise).
                result += currentOcclusion;
                count += 1.0;
            } else {
                result += currentOcclusion * 0.2;
                count += 0.2;
            }
        }
    }
    
    fragColor = result / count;
}
