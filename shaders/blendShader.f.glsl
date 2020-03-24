#version 330 core

struct Material {
    sampler2D texDiffuse0;
};
uniform Material material;

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;

out vec4 fragColor;

void main() {
    fragColor = texture(material.texDiffuse0, fTexCoords);
}
