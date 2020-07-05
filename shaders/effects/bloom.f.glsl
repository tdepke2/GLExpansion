#version 330 core

uniform sampler2D image;

in vec2 fTexCoords;

out vec4 fragColor;

void main() {
    vec3 color = texture(image, fTexCoords).rgb;
    
    if (dot(color, vec3(0.2126, 0.7152, 0.0722)) > 1.0) {    // Convert to grayscale and check if fragment above brightness threshold.
        fragColor = vec4(color, 1.0);
    } else {
        fragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
