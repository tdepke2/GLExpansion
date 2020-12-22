#version 330 core

uniform sampler2D texEquirectangular;

in vec3 fPosition;

out vec4 fragColor;

const vec2 INV_ATAN = vec2(0.1591, 0.3183);    // how tf was this calculated? #################################################
vec2 sampleSphericalMap(vec3 v) {
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= INV_ATAN;
    uv += 0.5;
    return uv;
}

void main() {
    vec2 uv = sampleSphericalMap(normalize(fPosition));
    vec3 color = texture(texEquirectangular, uv).rgb;
    
    fragColor = vec4(color, 1.0);
}
