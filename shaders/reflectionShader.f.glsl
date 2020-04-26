#version 330 core

uniform samplerCube skybox;
uniform vec3 cameraPos;

in vec3 fPosition;
in vec3 fNormal;

out vec4 fragColor;

void main() {
    //vec3 reflectVec = reflect(normalize(fPosition - cameraPos), normalize(fNormal));    // Reflective surface.
    vec3 reflectVec = refract(normalize(fPosition - cameraPos), normalize(fNormal), 1.0 / 1.52);    // Refractive surface.
    reflectVec.x = -reflectVec.x;
    fragColor = texture(skybox, reflectVec);
}
