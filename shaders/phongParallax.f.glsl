#version 330 core

const uint NUM_LIGHTS = 8u;
const uint DIRECTIONAL_LIGHT = 0u;
const uint POINT_LIGHT = 1u;
const uint SPOT_LIGHT = 2u;

struct Material {
    sampler2D texDiffuse0;
    sampler2D texSpecular0;
    sampler2D texNormal0;
    float shininess;
};
uniform Material material;

uniform vec3 viewPosition;
uniform bool lightStates[NUM_LIGHTS];

struct Light {
    uint type;
    vec3 position;           // Used in POINT_LIGHT and SPOT_LIGHT only.
    vec3 direction;          // Used in DIRECTIONAL_LIGHT and SPOT_LIGHT only.
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 attenuationVals;    // Used in POINT_LIGHT and SPOT_LIGHT only.
    vec2 cutOff;             // Used in SPOT_LIGHT only.
};
uniform Light lights[NUM_LIGHTS];

in vec3 fPosition;
in vec2 fTexCoords;
in mat3 fTBNMtx;

out vec4 fragColor;

vec3 calculateLight(Light light, vec3 normalDir, vec3 viewDir, vec3 diffuseColor, vec3 specularColor) {
    vec3 lightDir;
    float lightScalar;
    if (light.type == DIRECTIONAL_LIGHT) {
        lightDir = normalize(-light.direction);
        lightScalar = 1.0;
    } else {
        lightDir = normalize(light.position - fPosition);
        float distance = length(light.position - fPosition);
        lightScalar = 1.0 / (light.attenuationVals.x + light.attenuationVals.y * distance + light.attenuationVals.z * distance * distance);
    }
    
    vec3 ambient = light.ambient * diffuseColor * lightScalar;
    
    if (light.type == SPOT_LIGHT) {
        float theta = dot(lightDir, normalize(-light.direction));
        float intensity = clamp((theta - light.cutOff.y) / (light.cutOff.x - light.cutOff.y), 0.0, 1.0);
        lightScalar *= intensity;
    }
    
    float diffuseScalar = max(dot(normalDir, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diffuseScalar * diffuseColor * lightScalar;
    
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float specularScalar = (diffuseScalar == 0.0 ? 0.0 : pow(max(dot(normalDir, halfwayDir), 0.0), material.shininess));    // Blinn-Phong model.
    vec3 specular = light.specular * specularScalar * specularColor * lightScalar;
    
    return ambient + diffuse + specular;
}

void main() {
    vec4 diffuseColor = texture(material.texDiffuse0, fTexCoords);
    vec4 specularColor = texture(material.texSpecular0, fTexCoords).rrra;
    vec3 normalDir = normalize(fTBNMtx * (texture(material.texNormal0, fTexCoords).rgb * 2.0 - 1.0));
    vec3 viewDir = normalize(viewPosition - fPosition);
    
    vec3 color = vec3(0.0, 0.0, 0.0);
    for (uint i = 0u; i < NUM_LIGHTS; ++i) {
        if (lightStates[i]) {
            color += calculateLight(lights[i], normalDir, viewDir, vec3(diffuseColor), vec3(specularColor));
        }
    }
    
    fragColor = vec4(color, 1.0);
}
