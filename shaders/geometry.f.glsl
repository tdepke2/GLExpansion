#version 330 core

uniform sampler2D texDiffuse;
uniform sampler2D texSpecular;

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;

layout (location = 0) out vec3 position;
layout (location = 1) out vec3 normal;
layout (location = 2) out vec4 albedoSpec;

void main() {
    position = fPosition;
    normal = normalize(fNormal);
    albedoSpec.rgb = texture(texDiffuse, fTexCoords).rgb;
    albedoSpec.a = texture(texSpecular, fTexCoords).r;
}
