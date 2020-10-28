#version 330 core

uniform sampler2D texDiffuse;
uniform sampler2D texSpecular;
uniform sampler2D texNormal;

in vec3 fPosition;
in mat3 fTBNMtx;
in vec2 fTexCoords;

layout (location = 0) out vec3 position;
layout (location = 1) out vec3 normal;
layout (location = 2) out vec4 albedoSpec;

void main() {
    position = fPosition;
    normal = normalize(fTBNMtx * (texture(texNormal, fTexCoords).rgb * 2.0 - 1.0));
    albedoSpec = texture(texDiffuse, fTexCoords);
    if (albedoSpec.a < 0.1) {
        discard;
    }
    albedoSpec.a = texture(texSpecular, fTexCoords).r;
}
