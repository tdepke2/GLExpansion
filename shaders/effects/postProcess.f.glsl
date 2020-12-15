#version 330 core

const float GAMMA = 2.2;    // Gamma correction should match the implementation in World.cpp to compute light volumes correctly.

uniform sampler2D image;
uniform sampler2D bloomBlur;
uniform float exposure;
uniform bool applyBloom;

in vec2 fTexCoords;

out vec4 fragColor;

vec3 toneMapUncharted2(vec3 x) {
    const float A = 0.15;    // Shoulder strength.
    const float B = 0.50;    // Linear strength.
    const float C = 0.10;    // Linear angle.
    const float D = 0.20;    // Toe strength.
    const float E = 0.02;    // Toe numerator.
    const float F = 0.30;    // Toe denominator.
    
    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

void main() {
    vec3 hdrColor = texture(image, fTexCoords).rgb;
    if (applyBloom) {
        hdrColor += texture(bloomBlur, fTexCoords).rgb;    // Additive blending of image and bloom colors.
    }
    //vec3 mappedColor = hdrColor / (hdrColor + vec3(1.0));    // Reinhard tone mapping.
    //vec3 mappedColor = vec3(1.0) - exp(-hdrColor * exposure);    // Exposure tone mapping.
    vec3 mappedColor = toneMapUncharted2(hdrColor * exposure * 2.0) / toneMapUncharted2(vec3(11.2));    // Uncharted 2 tone mapping.
    
    fragColor = vec4(pow(mappedColor, vec3(1.0 / GAMMA)), 1.0);    // Apply gamma correction.
}
