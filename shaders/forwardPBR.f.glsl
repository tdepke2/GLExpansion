#version 330 core

const float PI = 3.14159265359;
const uint NUM_LIGHTS = 4u;
const uint DIRECTIONAL_LIGHT = 0u;
const uint POINT_LIGHT = 1u;
const uint SPOT_LIGHT = 2u;
const float AMBIENT = 0.03;
const float GAMMA = 2.2;

uniform sampler2D texAlbedo;
uniform sampler2D texMetallic;
uniform sampler2D texNormal;
uniform sampler2D texRoughness;
uniform sampler2D texAO;
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

float distributionGGX(float dotNH, float a) {
    float a2 = a * a;
    float denom = dotNH * dotNH * (a2 - 1.0) + 1.0;
    denom = PI * denom * denom;
    return a2 / denom;
}

float geometrySchlickGGX(float dotNV, float a) {
    float r = (a + 1.0);
    float k = (r * r) / 8.0;
    return dotNV / (dotNV * (1.0 - k) + k);
}

float geometrySmith(float dotNV, float dotNL, float a) {
    float ggx2 = geometrySchlickGGX(dotNV, a);
    float ggx1 = geometrySchlickGGX(dotNL, a);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float dotHV, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - dotHV, 0.0, 1.0), 5.0);
}

void main() {
    vec3 albedo = texture(texAlbedo, fTexCoords).rgb;
    float metallic = texture(texMetallic, fTexCoords).r;
    vec3 N = normalize(fTBNMtx * (texture(texNormal, fTexCoords).rgb * 2.0 - 1.0));
    float roughness = texture(texRoughness, fTexCoords).r;
    float ambientOcclusion = texture(texAO, fTexCoords).r;
    vec3 V = normalize(-fPosition);
    float dotNV = max(dot(N, V), 0.0);
    
    vec3 F0 = vec3(0.04);    // Initialize surface reflection at zero incidence to the average of dielectric (non-metallic) surfaces.
    F0 = mix(F0, albedo, metallic);
    
    vec3 irradiance = vec3(0.0);    // Reflectance equation.
    for (uint i = 0u; i < NUM_LIGHTS; ++i) {
        if (lightStates[i] && lights[i].type == POINT_LIGHT) {
            vec3 L = normalize(lights[i].positionViewSpace - fPosition);
            vec3 H = normalize(V + L);
            float dotNL = max(dot(N, L), 0.0);
            float distanceFragToLight = length(lights[i].positionViewSpace - fPosition);
            float attenuation = 1.0 / (distanceFragToLight * distanceFragToLight);
            vec3 radiance = lights[i].diffuse * attenuation;
            
            // Cook-Torrance BRDF.
            float NDF = distributionGGX(max(dot(N, H), 0.0), roughness * roughness);
            float G = geometrySmith(dotNV, dotNL, roughness * roughness);
            vec3 F = fresnelSchlick(dot(H, V), F0);
            
            vec3 kD = vec3(1.0) - F;    // The diffuse light component is the remaining light after specular (given by Fresnel) leaves the surface.
            kD *= 1.0 - metallic;    // Metallic surfaces absorb the diffuse light.
            vec3 specular = NDF * G * F / max(4.0 * dotNV * dotNL, 0.001);    // Compute finalized Cook-Torrance specular (includes kS from Fresnel equation).
            
            irradiance += (kD * albedo / PI + specular) * radiance * dotNL;
        }
    }
    
    vec3 color = vec3(AMBIENT) * albedo * ambientOcclusion + irradiance;
    
    vec3 mappedColor = color / (color + vec3(1.0));    // Reinhard tone mapping.
    fragColor = vec4(pow(mappedColor, vec3(1.0 / GAMMA)), 1.0);    // Apply gamma correction.
}
