#version 330 core

uniform vec3 lightColor;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 bloomColor;

void main() {
    fragColor = vec4(lightColor, 1.0);
    if (dot(fragColor.rgb, vec3(0.2126, 0.7152, 0.0722)) > 1.0) {    // Convert to grayscale and check if fragment above brightness threshold.
        bloomColor = fragColor;
    } else {
        bloomColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
