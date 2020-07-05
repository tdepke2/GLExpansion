#version 330 core

const uint NUM_LIGHTS = 8u;
const uint DIRECTIONAL_LIGHT = 0u;
const uint POINT_LIGHT = 1u;
const uint SPOT_LIGHT = 2u;

uniform sampler2D texPosition;
uniform sampler2D texNormal;
uniform sampler2D texAlbedoSpec;
uniform sampler2D texSSAO;
uniform sampler2D shadowMap;
uniform bool applySSAO;
uniform mat4 viewToLightSpaceMtx;
uniform bool lightStates[NUM_LIGHTS];

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

in vec2 fTexCoords;

out vec4 fragColor;

float calculateShadow(vec4 positionLightSpace, vec3 normal, vec3 lightDir) {
    vec3 normalizedDeviceCoords = (positionLightSpace.xyz / positionLightSpace.w) * 0.5 + 0.5;
    float shadowBias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
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

vec3 calculateLight(Light light, vec3 position, vec3 normal, vec3 diffuseColor, float specularColor, float ambientOcclusion, vec3 viewDir) {    // Computes the color of a fragment with one light source. All positions/directions in view space.
    vec3 lightDir;
    float lightScalar;
    if (light.type == DIRECTIONAL_LIGHT) {
        lightDir = normalize(-light.directionViewSpace);
        lightScalar = 1.0;
    } else {
        lightDir = normalize(light.positionViewSpace - position);
        float distance = length(light.positionViewSpace - position);
        lightScalar = 1.0 / (light.attenuationVals.x + light.attenuationVals.y * distance + light.attenuationVals.z * distance * distance);
    }
    
    vec3 ambient = light.ambient * diffuseColor * lightScalar * ambientOcclusion;
    
    if (light.type == SPOT_LIGHT) {
        float theta = dot(lightDir, normalize(-light.directionViewSpace));
        float intensity = clamp((theta - light.cutOff.y) / (light.cutOff.x - light.cutOff.y), 0.0, 1.0);
        lightScalar *= intensity;
    } else if (light.type == DIRECTIONAL_LIGHT) {
        lightScalar *= calculateShadow(viewToLightSpaceMtx * vec4(position, 1.0), normal, lightDir);
    }
    
    float diffuseScalar = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diffuseScalar * diffuseColor * lightScalar;
    
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float specularScalar = (diffuseScalar == 0.0 ? 0.0 : pow(max(dot(normal, halfwayDir), 0.0), 64.0));    // Blinn-Phong model.
    vec3 specular = light.specular * specularScalar * specularColor * lightScalar;
    
    return ambient + diffuse + specular;
}

void main() {
    vec3 position = texture(texPosition, fTexCoords).rgb;
    vec3 normal = texture(texNormal, fTexCoords).rgb;
    vec3 diffuseColor = texture(texAlbedoSpec, fTexCoords).rgb;
    float specularColor = texture(texAlbedoSpec, fTexCoords).a;
    float ambientOcclusion = (applySSAO ? texture(texSSAO, fTexCoords).r : 1.0);
    vec3 viewDir = normalize(-position);
    
    vec3 color = vec3(0.0, 0.0, 0.0);
    for (uint i = 0u; i < NUM_LIGHTS; ++i) {
        if (lightStates[i]) {
            color += calculateLight(lights[i], position, normal, diffuseColor, specularColor, ambientOcclusion, viewDir);
        }
    }
    
    //color += vec3(ambientOcclusion);    // Override color to visualize AO #################################################################################
    fragColor = vec4(color, 1.0);
}
