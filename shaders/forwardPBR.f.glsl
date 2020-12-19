#version 330 core

const float PI = 3.14159265359;
const uint NUM_LIGHTS = 4u;
const uint DIRECTIONAL_LIGHT = 0u;
const uint POINT_LIGHT = 1u;
const uint SPOT_LIGHT = 2u;
const float AMBIENT = 0.03;
const float GAMMA = 2.2;

uniform sampler2D texDiffuse;
uniform sampler2D texSpecular;
uniform sampler2D texNormal;
uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
uniform float ambientOcclusion;
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

in vec3 fPosition;
in mat3 fTBNMtx;
in vec2 fTexCoords;

out vec4 fragColor;

vec3 calculateLight(Light light, vec3 normal, vec3 viewDir, vec3 diffuseColor, float specularColor) {    // Computes the color of a fragment with one light source. All positions/directions in view space.
    vec3 lightDir;
    float lightScalar;
    if (light.type == DIRECTIONAL_LIGHT) {
        lightDir = normalize(-light.directionViewSpace);
        lightScalar = 1.0;
    } else {
        lightDir = normalize(light.positionViewSpace - fPosition);
        float distanceFragToLight = length(light.positionViewSpace - fPosition);
        lightScalar = 1.0 / (light.attenuationVals.x + light.attenuationVals.y * distanceFragToLight + light.attenuationVals.z * distanceFragToLight * distanceFragToLight);
    }
    
    vec3 ambient = light.ambient * diffuseColor * lightScalar;
    
    if (light.type == SPOT_LIGHT) {
        float theta = dot(lightDir, normalize(-light.directionViewSpace));
        float intensity = clamp((theta - light.cutOff.y) / (light.cutOff.x - light.cutOff.y), 0.0, 1.0);
        lightScalar *= intensity;
    } else if (light.type == DIRECTIONAL_LIGHT) {
        //lightScalar *= calculateShadow(fPositionLightSpace, normalDir, lightDir);
    }
    
    float diffuseScalar = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diffuseScalar * diffuseColor * lightScalar;
    
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float specularScalar = (diffuseScalar == 0.0 ? 0.0 : pow(max(dot(normal, halfwayDir), 0.0), 64.0));    // Blinn-Phong model.
    vec3 specular = light.specular * specularScalar * specularColor * lightScalar;
    
    return ambient + diffuse + specular;
}

float distributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float dotNH = max(dot(N, H), 0.0);
    float dotNH2 = dotNH * dotNH;
    
    float denom = (dotNH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return a2 / denom;
}

float geometrySchlickGGX(float dotNV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    return dotNV / (dotNV * (1.0 - k) + k);
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float dotNV = max(dot(N, V), 0.0);
    float dotNL = max(dot(N, L), 0.0);
    float ggx2 = geometrySchlickGGX(dotNV, roughness);
    float ggx1 = geometrySchlickGGX(dotNL, roughness);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    /*vec4 diffuseColor = texture(texDiffuse, fTexCoords);
    //if (diffuseColor.a < 0.5) {
        //discard;
    //}
    float specularColor = texture(texSpecular, fTexCoords).r;
    vec3 normal = normalize(fTBNMtx * (texture(texNormal, fTexCoords).rgb * 2.0 - 1.0));
    vec3 viewDir = normalize(-fPosition);
    
    vec3 color = vec3(0.0, 0.0, 0.0);
    for (uint i = 0u; i < NUM_LIGHTS; ++i) {
        if (lightStates[i]) {
            color += calculateLight(lights[i], normal, viewDir, vec3(diffuseColor), specularColor);
        }
    }*/
    
    vec3 N = normalize(fTBNMtx * (texture(texNormal, fTexCoords).rgb * 2.0 - 1.0));
    vec3 V = normalize(-fPosition);
    
    vec3 F0 = vec3(0.04);    // Initialize surface reflection at zero incidence to the average of dielectric (non-metallic) surfaces.
    F0 = mix(F0, albedo, metallic);
    
    vec3 irradiance = vec3(0.0);    // Reflectance equation.
    for (uint i = 0u; i < NUM_LIGHTS; ++i) {
        if (lightStates[i] && lights[i].type == POINT_LIGHT) {
            vec3 L = normalize(lights[i].positionViewSpace - fPosition);
            vec3 H = normalize(V + L);
            float distanceFragToLight = length(lights[i].positionViewSpace - fPosition);
            float attenuation = 1.0 / (distanceFragToLight * distanceFragToLight);
            vec3 radiance = lights[i].diffuse * attenuation;
            
            // Cook-Torrance BRDF.
            float NDF = distributionGGX(N, H, roughness);
            float G = geometrySmith(N, V, L, roughness);
            vec3 F = fresnelSchlick(dot(H, V), F0);
            
            vec3 kS = F;
            vec3 kD = vec3(1.0) - kS;
            kD *= 1.0 - metallic;
            vec3 specular = NDF * G * F / max(4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0), 0.001);
            
            float dotNL = max(dot(N, L), 0.0);
            irradiance += (kD * albedo / PI + specular) * radiance * dotNL;
        }
    }
    
    vec3 color = vec3(AMBIENT) * albedo * ambientOcclusion + irradiance;
    
    vec3 mappedColor = color / (color + vec3(1.0));    // Reinhard tone mapping.
    fragColor = vec4(pow(mappedColor, vec3(1.0 / GAMMA)), 1.0);    // Apply gamma correction.
}
