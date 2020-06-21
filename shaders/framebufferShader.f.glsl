#version 330 core

const float GAMMA = 2.2;

const float OFFSET = 1.0 / 300.0;
const vec2 OFFSETS[9] = vec2[](
    vec2( OFFSET,  OFFSET),
    vec2(   0.0f,  OFFSET),
    vec2(-OFFSET,  OFFSET),
    vec2( OFFSET,    0.0f),
    vec2(   0.0f,    0.0f),
    vec2(-OFFSET,    0.0f),
    vec2( OFFSET, -OFFSET),
    vec2(   0.0f, -OFFSET),
    vec2(-OFFSET, -OFFSET)
);

const float KERNEL[9] = float[](
    1.0,  1.0,  1.0,
    1.0, -8.0,  1.0,
    1.0,  1.0,  1.0
);

uniform sampler2D tex;
uniform float exposure;

in vec2 fTexCoords;

out vec4 fragColor;

void main() {
    vec3 hdrColor = texture(tex, fTexCoords).rgb;
    //vec3 mappedColor = hdrColor / (hdrColor + vec3(1.0));    // Reinhard tone mapping.
    vec3 mappedColor = vec3(1.0) - exp(-hdrColor * exposure);    // Exposure tone mapping.
    fragColor = vec4(pow(mappedColor, vec3(1.0 / GAMMA)), 1.0);    // Apply gamma correction.
    
    //vec3 sum = vec3(0.0);
    //for (int i = 0; i < 9; ++i) {
        //sum += vec3(texture(tex, fTexCoords.st + OFFSETS[i])) * KERNEL[i];
    //}
    //fragColor = vec4(sum, 1.0);
}
