#version 330 core

const uint NUM_POINT_LIGHTS = 4u;

//uniform bool lightStates[NUM_LIGHTS];

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};
uniform Material material;

struct DirectionalLight {
    vec3 directionViewSpace;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform DirectionalLight directionalLight;

struct PointLight {
    vec3 positionViewSpace;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 attenuationVals;
};
uniform PointLight pointLights[NUM_POINT_LIGHTS];

struct SpotLight {
    vec3 positionViewSpace;
    vec3 directionViewSpace;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 attenuationVals;
    vec2 cutOff;
};
uniform SpotLight spotLight;

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;

out vec4 fragColor;

vec3 calculateDirectionalLight(DirectionalLight light, vec3 normalDir, vec3 viewDir) {
    vec3 lightDir = normalize(-light.directionViewSpace);
    vec3 reflectDir = reflect(-lightDir, normalDir);
    //vec3 halfwayDir = normalize(lightDir + viewDir);
    
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, fTexCoords));
    
    float diffuseScalar = max(dot(normalDir, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diffuseScalar * vec3(texture(material.diffuse, fTexCoords));
    
    float specularScalar = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);    // Blinn model.
    vec3 specular = light.specular * specularScalar * vec3(texture(material.specular, fTexCoords));
    
    //float specularScalar = pow(max(dot(normalDir, halfwayDir), 0.0), material.shininess);    // Blinn-Phong model.
    //vec3 specular = light.specular * specularScalar * vec3(texture(material.specular, fTexCoords));
    
    return ambient + diffuse + specular;
}

vec3 calculatePointLight(PointLight light, vec3 position, vec3 normalDir, vec3 viewDir) {
    vec3 lightDir = normalize(light.positionViewSpace - position);
    vec3 reflectDir = reflect(-lightDir, normalDir);
    //vec3 halfwayDir = normalize(lightDir + viewDir);
    float distance = length(light.positionViewSpace - position);
    float attenuation = 1.0 / (light.attenuationVals.x + light.attenuationVals.y * distance + light.attenuationVals.z * distance * distance);
    
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, fTexCoords)) * attenuation;
    
    float diffuseScalar = max(dot(normalDir, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diffuseScalar * vec3(texture(material.diffuse, fTexCoords)) * attenuation;
    
    float specularScalar = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);    // Blinn model.
    vec3 specular = light.specular * specularScalar * vec3(texture(material.specular, fTexCoords)) * attenuation;
    
    //float specularScalar = pow(max(dot(normalDir, halfwayDir), 0.0), material.shininess);    // Blinn-Phong model.
    //vec3 specular = light.specular * specularScalar * vec3(texture(material.specular, fTexCoords)) * attenuation;
    
    return ambient + diffuse + specular;
}

vec3 calculateSpotLight(SpotLight light, vec3 position, vec3 normalDir, vec3 viewDir) {
    vec3 lightDir = normalize(light.positionViewSpace - position);
    vec3 reflectDir = reflect(-lightDir, normalDir);
    //vec3 halfwayDir = normalize(lightDir + viewDir);
    float distance = length(light.positionViewSpace - position);
    float attenuation = 1.0 / (light.attenuationVals.x + light.attenuationVals.y * distance + light.attenuationVals.z * distance * distance);
    
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, fTexCoords)) * attenuation;
    
    float theta = dot(lightDir, normalize(-light.directionViewSpace));
    float intensity = min((theta - light.cutOff.y) / (light.cutOff.x - light.cutOff.y), 1.0);
    if (intensity <= 0.0) {
        return ambient;
    }
    
    float diffuseScalar = max(dot(normalDir, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diffuseScalar * vec3(texture(material.diffuse, fTexCoords)) * intensity * attenuation;
    
    float specularScalar = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);    // Blinn model.
    vec3 specular = light.specular * specularScalar * vec3(texture(material.specular, fTexCoords)) * intensity * attenuation;
    
    //float specularScalar = pow(max(dot(normalDir, halfwayDir), 0.0), material.shininess);    // Blinn-Phong model.
    //vec3 specular = light.specular * specularScalar * vec3(texture(material.specular, fTexCoords)) * intensity * attenuation;
    
    return ambient + diffuse + specular;
}

void main() {
    vec3 normalDir = normalize(fNormal);
    vec3 viewDir = normalize(-fPosition);
    
    vec3 color = calculateDirectionalLight(directionalLight, normalDir, viewDir);
    
    for (uint i = 0u; i < NUM_POINT_LIGHTS; ++i) {
        color += calculatePointLight(pointLights[i], fPosition, normalDir, viewDir);
    }
    
    color += calculateSpotLight(spotLight, fPosition, normalDir, viewDir);
    
    fragColor = vec4(color, 1.0);
}
