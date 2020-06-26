#version 330 core

const float GAMMA = 2.2;

uniform sampler2D image;
uniform sampler2D bloomBlur;
uniform float exposure;

in vec2 fTexCoords;

out vec4 fragColor;

void main() {
    vec3 hdrColor = texture(image, fTexCoords).rgb + texture(bloomBlur, fTexCoords).rgb;    // Additive blending of image and bloom colors.
    //vec3 mappedColor = hdrColor / (hdrColor + vec3(1.0));    // Reinhard tone mapping.
    vec3 mappedColor = vec3(1.0) - exp(-hdrColor * exposure);    // Exposure tone mapping.
    fragColor = vec4(pow(mappedColor, vec3(1.0 / GAMMA)), 1.0);    // Apply gamma correction.
}
