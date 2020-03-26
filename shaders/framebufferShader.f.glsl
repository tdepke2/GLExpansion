#version 330 core

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
    1.0, 1.0, 1.0,
    1.0, -8.0, 1.0,
    1.0, 1.0, 1.0
);

uniform sampler2D tex;

in vec2 fTexCoords;

out vec4 fragColor;

void main() {
    //fragColor = texture(tex, fTexCoords);
    
    vec3 sum = vec3(0.0);
    for (int i = 0; i < 9; ++i) {
        sum += vec3(texture(tex, fTexCoords.st + OFFSETS[i])) * KERNEL[i];
    }
    fragColor = vec4(sum, 1.0);
}
