#version 330 core

const uint NUM_LIGHTS = 64u;
const uint DIRECTIONAL_LIGHT = 0u;
const uint POINT_LIGHT = 1u;
const uint SPOT_LIGHT = 2u;
const uint NUM_CASCADED_SHADOWS = 3u;
const float SHADOW_BLUR_BAND = 1.0;

uniform sampler2D texPosition;
uniform sampler2D texNormal;
uniform sampler2D texAlbedoSpec;
uniform sampler2D texSSAO;
uniform bool applySSAO;
uniform sampler2DShadow shadowMap[NUM_CASCADED_SHADOWS];
uniform mat4 viewToLightSpace[NUM_CASCADED_SHADOWS];
uniform float shadowZEnds[NUM_CASCADED_SHADOWS];
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

float calculateShadow(uint cascadeIndex, vec3 position, vec3 normal, vec3 lightDir) {
    vec4 positionLightSpace = viewToLightSpace[cascadeIndex] * vec4(position, 1.0);
    vec3 normalizedDeviceCoords = (positionLightSpace.xyz / positionLightSpace.w) * 0.5 + 0.5;
    float shadowBias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.0001);
    float brightness = 0.0;
    if (normalizedDeviceCoords.z > 1.0) {
        brightness = 1.0;
    } else {
        vec2 texelSize = 1.0 / textureSize(shadowMap[cascadeIndex], 0);    // Apply percentage-closer filtering for softer shadows.
        for (int y = -1; y <= 1; ++y) {
            for (int x = -1; x <= 1; ++x) {
                brightness += texture(shadowMap[cascadeIndex], vec3(normalizedDeviceCoords.xy + vec2(x, y) * texelSize, normalizedDeviceCoords.z));    // Front face culling method (works with most geometry but has light bleed issues).
                //brightness += texture(shadowMap[cascadeIndex], vec3(normalizedDeviceCoords.xy + vec2(x, y) * texelSize, normalizedDeviceCoords.z - shadowBias));    // Back face culling method (requires euclidean geometry but gives better results).
            }
        }
        brightness /= 9.0;
        
        // Hard shadows without PCF (use with sampler2D only).
        //brightness = (normalizedDeviceCoords.z - shadowBias > texture(shadowMap[cascadeIndex], normalizedDeviceCoords.xy).r) ? 0.0 : 1.0;
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
        uint cascadeIndex = 0u;
        for (uint i = 0u; i < NUM_CASCADED_SHADOWS - 1u; ++i) {
            cascadeIndex += (-position.z > shadowZEnds[i]) ? 1u : 0u;
        }
        
        /*if (cascadeIndex == 0u) {    // Draw color for each cascade.
            tempColor = vec3(1.0, 0.5, 0.5);
        } else if (cascadeIndex == 1u) {
            tempColor = vec3(0.5, 1.0, 0.5);
        } else if (cascadeIndex == 2u) {
            tempColor = vec3(0.5, 0.5, 1.0);
        }*/
        
        if (cascadeIndex == NUM_CASCADED_SHADOWS - 1u || shadowZEnds[cascadeIndex] + position.z > SHADOW_BLUR_BAND) {    // Blur between cascades to remove seam between shadow maps.
            lightScalar *= calculateShadow(cascadeIndex, position, normal, lightDir);
        } else {
            lightScalar *= mix(calculateShadow(cascadeIndex + 1u, position, normal, lightDir), calculateShadow(cascadeIndex, position, normal, lightDir), (shadowZEnds[cascadeIndex] + position.z) / SHADOW_BLUR_BAND);
        }
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
