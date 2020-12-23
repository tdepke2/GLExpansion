#version 330 core

const float PI = 3.14159265359;

uniform sampler2D texEquirectangular;

in vec3 fPosition;

out vec4 fragColor;

vec2 sampleSphericalMap(vec3 v) {    // Based on finding UV on a sphere. https://en.wikipedia.org/wiki/UV_mapping
    return vec2(0.5 + atan(v.z, v.x) / (2.0 * PI), 0.5 + asin(v.y) / PI);
}

void main() {
    vec2 uv = sampleSphericalMap(normalize(vec3(fPosition.xy, -fPosition.z)));    // Negate Z to apply correction to cubemap.
    vec3 color = texture(texEquirectangular, uv).rgb;
    
    fragColor = vec4(color, 1.0);
}
