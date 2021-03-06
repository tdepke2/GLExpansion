#version 330 core

const uint NUM_LIGHTS = 8u;
const uint DIRECTIONAL_LIGHT = 0u;
const uint POINT_LIGHT = 1u;
const uint SPOT_LIGHT = 2u;

uniform bool lightStates[NUM_LIGHTS];

struct Material {
    sampler2D texDiffuse0;
    sampler2D texSpecular0;
    float shininess;
};
uniform Material material;
uniform sampler2D shadowMap;

struct Light {
    uint type;
    vec3 positionViewSpace;     // Used in POINT_LIGHT and SPOT_LIGHT only.
    vec3 directionViewSpace;    // Used in DIRECTIONAL_LIGHT and SPOT_LIGHT only.
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 attenuationVals;       // Used in POINT_LIGHT and SPOT_LIGHT only.
    vec2 cutOff;                // Used in SPOT_LIGHT only.
};
uniform Light lights[NUM_LIGHTS];

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;
in vec4 fPositionLightSpace;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 bloomColor;

float calculateShadow(vec4 positionLightSpace, vec3 normalDir, vec3 lightDir) {
    vec3 normalizedDeviceCoords = (positionLightSpace.xyz / positionLightSpace.w) * 0.5 + 0.5;
    float shadowBias = max(0.05 * (1.0 - dot(normalDir, lightDir)), 0.005);
    float brightness = 0.0;
    if (normalizedDeviceCoords.z > 1.0) {
        brightness = 1.0;
    } else {
        vec2 texelSize = 1.0 / textureSize(shadowMap, 0);    // Apply percentage-closer filtering for softer shadows.
        for (int y = -1; y <= 1; ++y) {
            for (int x = -1; x <= 1; ++x) {
                brightness += normalizedDeviceCoords.z > texture(shadowMap, normalizedDeviceCoords.xy + vec2(x, y) * texelSize).r ? 0.0 : 1.0;
            }
        }
        brightness /= 9.0;
    }
    return brightness;
}

vec3 calculateLight(Light light, vec3 normalDir, vec3 viewDir, vec3 diffuseColor, vec3 specularColor) {
    vec3 lightDir;
    float lightScalar;
    if (light.type == DIRECTIONAL_LIGHT) {
        lightDir = normalize(-light.directionViewSpace);
        lightScalar = 1.0;
    } else {
        lightDir = normalize(light.positionViewSpace - fPosition);
        float distance = length(light.positionViewSpace - fPosition);
        lightScalar = 1.0 / (light.attenuationVals.x + light.attenuationVals.y * distance + light.attenuationVals.z * distance * distance);
    }
    //vec3 reflectDir = reflect(-lightDir, normalDir);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    
    vec3 ambient = light.ambient * diffuseColor * lightScalar;
    
    if (light.type == SPOT_LIGHT) {
        float theta = dot(lightDir, normalize(-light.directionViewSpace));
        float intensity = min((theta - light.cutOff.y) / (light.cutOff.x - light.cutOff.y), 1.0);
        if (intensity <= 0.0) {
            return ambient;
        }
        lightScalar *= intensity;
    } else if (light.type == DIRECTIONAL_LIGHT) {
        lightScalar *= calculateShadow(fPositionLightSpace, normalDir, lightDir);
    }
    
    float diffuseScalar = max(dot(normalDir, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diffuseScalar * diffuseColor * lightScalar;
    
    //float specularScalar = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);    // Blinn model.
    //vec3 specular = light.specular * specularScalar * specularColor * lightScalar;
    
    float specularScalar = (diffuseScalar == 0.0 ? 0.0 : pow(max(dot(normalDir, halfwayDir), 0.0), material.shininess));    // Blinn-Phong model.
    vec3 specular = light.specular * specularScalar * specularColor * lightScalar;
    
    return ambient + diffuse + specular;
}

void main() {
    vec4 diffuseColor = texture(material.texDiffuse0, fTexCoords);
    //if (diffuseColor.a < 0.5) {
        //discard;
    //}
    vec4 specularColor = texture(material.texSpecular0, fTexCoords).rrra;
    vec3 normalDir = normalize(fNormal);
    vec3 viewDir = normalize(-fPosition);
    
    vec3 color = vec3(0.0, 0.0, 0.0);
    for (uint i = 0u; i < NUM_LIGHTS; ++i) {
        if (lightStates[i]) {
            color += calculateLight(lights[i], normalDir, viewDir, vec3(diffuseColor), vec3(specularColor));
        }
    }
    
    fragColor = vec4(color, 1.0);
    if (dot(fragColor.rgb, vec3(0.2126, 0.7152, 0.0722)) > 1.0) {    // Convert to grayscale and check if fragment above brightness threshold.
        bloomColor = fragColor;
    } else {
        bloomColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
