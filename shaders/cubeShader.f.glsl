#version 330 core

uniform sampler2D tex1;
uniform sampler2D tex2;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};
uniform Material material;

struct Light {
    vec3 positionViewSpace;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 attenuationVals;
};
uniform Light light1;

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoord;

out vec4 fragColor;

vec3 calculateFragmentLight(Light light, vec3 position, vec3 normalDir, vec3 viewDir, float brightness) {
    vec3 lightDir = normalize(light.positionViewSpace - position);
    vec3 reflectDir = reflect(-lightDir, normalDir);
    //vec3 halfwayDir = normalize(lightDir + viewDir);
    //float distance = length(light.positionViewSpace - position);
    //float attenuation = 1.0 / (light.attenuationVals.x + light.attenuationVals.y * distance + light.attenuationVals.z * distance * distance);
    
    vec3 ambient = light.ambient * material.ambient;// * attenuation;
    vec3 diffuse = light.diffuse * (max(dot(normalDir, lightDir), 0.0) * material.diffuse);// * attenuation;
    vec3 specular = light.specular * (pow(max(dot(viewDir, reflectDir), 0.0), material.shininess) * material.specular);// * attenuation;    // Blinn model.
    //vec3 specular = light.specular * (pow(max(dot(normalDir, halfwayDir), 0.0), material.shininess) * material.specular);// * attenuation;    // Blinn-Phong model.
    
    return ambient + brightness * (diffuse + specular);
}

void main() {
    vec3 normalDir = normalize(fNormal);
    vec3 viewDir = normalize(-fPosition);
    
    fragColor = vec4(calculateFragmentLight(light1, fPosition, normalDir, viewDir, 1.0), 1.0);
}
