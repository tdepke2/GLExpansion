#version 330 core

const float NEAR_PLANE = 0.1f, FAR_PLANE = 100.0f;

out vec4 FragColor;

float linearizeDepth(float depth) {
    float z = depth * 2.0 - 1.0;    // Convert to normalized device coords.
    return (2.0 * FAR_PLANE * NEAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));
}

void main() {
    FragColor = vec4(vec3(linearizeDepth(gl_FragCoord.z) / FAR_PLANE), 1.0);
}
