#version 330 core

uniform sampler2D image;
uniform bool blurHorizontal;
uniform float weights[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

in vec2 fTexCoords;

out vec4 fragColor;

void main() {
    vec2 texelSize = 1.0 / textureSize(image, 0);
    vec3 color = texture(image, fTexCoords).rgb * weights[0];
    if (blurHorizontal) {
        for (int i = 1; i < 5; ++i) {
            color += texture(image, fTexCoords + vec2(texelSize.x * i, 0.0)).rgb * weights[i];
            color += texture(image, fTexCoords - vec2(texelSize.x * i, 0.0)).rgb * weights[i];
        }
    } else {
        for (int i = 1; i < 5; ++i) {
            color += texture(image, fTexCoords + vec2(0.0, texelSize.y * i)).rgb * weights[i];
            color += texture(image, fTexCoords - vec2(0.0, texelSize.y * i)).rgb * weights[i];
        }
    }
    fragColor = vec4(color, 1.0);
}
