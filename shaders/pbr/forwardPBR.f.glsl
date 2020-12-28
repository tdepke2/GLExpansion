#version 330 core

const float PI = 3.14159265359;
const uint NUM_LIGHTS = 4u;
const uint DIRECTIONAL_LIGHT = 0u;
const uint POINT_LIGHT = 1u;
const uint SPOT_LIGHT = 2u;
const float GAMMA = 2.2;
const float MAX_REFLECTION_LOD = 4.0;

layout (std140) uniform ViewProjectionMtx {
    uniform mat4 viewMtx;
    uniform mat4 projectionMtx;
};
uniform bool lightStates[NUM_LIGHTS];
uniform sampler2D texAlbedo;
uniform sampler2D texMetallic;
uniform sampler2D texNormal;
uniform sampler2D texRoughness;
uniform sampler2D texAO;
uniform samplerCube irradianceCubemap;
uniform samplerCube prefilterCubemap;
uniform sampler2D lookupBRDF;

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

float distributionGGX(float dotNH, float alpha) {    // Normal distribution function used to approximate surface microfacets (Trowbridge-Reitz GGX).
    float alpha2 = alpha * alpha;
    float x = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
    return alpha2 / (PI * x * x);
}

float geometrySchlickGGXAL(float dotNV, float roughness) {    // Partial geometry function for analytical lights (Schlick-GGX).
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    return dotNV / (dotNV * (1.0 - k) + k);
}

float geometrySmithAL(float dotNV, float dotNL, float roughness) {    // Geometry function used to approximate surface shadowing for analytical lights (Smith's method).
    float ggx2 = geometrySchlickGGXAL(dotNV, roughness);
    float ggx1 = geometrySchlickGGXAL(dotNL, roughness);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {    // Fresnel equation used to determine percentage of light that gets reflected (Fresnel-Schlick).
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {    // Alternate Fresnel equation for improved reflections on rough surfaces with IBL.
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    vec3 albedo = texture(texAlbedo, fTexCoords).rgb;
    float metallic = texture(texMetallic, fTexCoords).r;
    vec3 N = normalize(fTBNMtx * (texture(texNormal, fTexCoords).rgb * 2.0 - 1.0));
    float roughness = texture(texRoughness, fTexCoords).r;
    float ambientOcclusion = texture(texAO, fTexCoords).r;
    vec3 V = normalize(-fPosition);
    float dotNV = max(dot(N, V), 0.0);
    float alpha = roughness * roughness;
    
    vec3 F0 = vec3(0.04);    // Initialize surface reflection at zero incidence to the average of dielectric (non-metallic) surfaces.
    F0 = mix(F0, albedo, metallic);
    
    vec3 radianceOut = vec3(0.0);    // Reflectance equation.
    for (uint i = 0u; i < NUM_LIGHTS; ++i) {
        if (lightStates[i] && lights[i].type == POINT_LIGHT) {
            vec3 L = normalize(lights[i].positionViewSpace - fPosition);
            vec3 H = normalize(V + L);
            float dotNL = max(dot(N, L), 0.0);
            float distanceFragToLight = length(lights[i].positionViewSpace - fPosition);
            float attenuation = 1.0 / (distanceFragToLight * distanceFragToLight);
            vec3 radiance = lights[i].diffuse * attenuation;
            
            float NDF = distributionGGX(max(dot(N, H), 0.0), alpha);    // Cook-Torrance BRDF.
            float G = geometrySmithAL(dotNV, dotNL, roughness);
            vec3 F = fresnelSchlick(dot(H, V), F0);
            
            vec3 kD = vec3(1.0) - F;    // The diffuse light component is the remaining light after specular (given by Fresnel) leaves the surface.
            kD *= 1.0 - metallic;    // Metallic surfaces absorb the diffuse light.
            vec3 specular = NDF * G * F / max(4.0 * dotNV * dotNL, 0.001);    // Compute finalized Cook-Torrance specular (includes kS from Fresnel equation).
            // check above, the bias may need to be directly added ######################################
            radianceOut += (kD * albedo / PI + specular) * radiance * dotNL;
        }
    }
    
    vec3 kS = fresnelSchlickRoughness(dot(N, V), F0, roughness);    // Use IBL to compute ambient lighting component.
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;
    vec3 normalWorldSpace = transpose(mat3(viewMtx)) * N;
    normalWorldSpace.z = -normalWorldSpace.z;
    vec3 irradiance = texture(irradianceCubemap, normalWorldSpace).rgb;
    vec3 diffuse = irradiance * albedo;
    
    vec3 reflectionWorldSpace = transpose(mat3(viewMtx)) * reflect(-V, N);
    reflectionWorldSpace.z = -reflectionWorldSpace.z;
    vec3 prefilteredColor = textureLod(prefilterCubemap, reflectionWorldSpace, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 resultBRDF = texture(lookupBRDF, vec2(dotNV, roughness)).rg;
    vec3 specular = prefilteredColor * (kS * resultBRDF.x + resultBRDF.y);
    
    vec3 ambient = (kD * diffuse + specular) * ambientOcclusion;
    vec3 color = ambient + radianceOut;
    
    vec3 mappedColor = color / (color + vec3(1.0));    // Reinhard tone mapping.
    fragColor = vec4(pow(mappedColor, vec3(1.0 / GAMMA)), 1.0);    // Apply gamma correction.
}
