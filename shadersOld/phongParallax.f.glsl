#version 330 core

const uint NUM_LIGHTS = 8u;
const uint DIRECTIONAL_LIGHT = 0u;
const uint POINT_LIGHT = 1u;
const uint SPOT_LIGHT = 2u;
const float MIN_PARALLAX_LAYERS = 8.0;
const float MAX_PARALLAX_LAYERS = 32.0;

struct Material {
    sampler2D texDiffuse0;
    sampler2D texSpecular0;
    sampler2D texNormal0;
    sampler2D texDisplacement0;
    float shininess;
    float heightScale;
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

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 bloomColor;

vec2 mapParallax(vec2 texCoords, vec3 viewDir) {    // Applies parallax to a texture coordinate given the current view and displacement map. All positions/directions in tangent space.
    float numLayers = mix(MAX_PARALLAX_LAYERS, MIN_PARALLAX_LAYERS, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));
    float layerDepth = 0.0;
    float deltaLayerDepth = 1.0 / numLayers;
    vec2 parallax = viewDir.xy / viewDir.z * material.heightScale;
    vec2 deltaTexCoords = -parallax / numLayers;
    float depth = texture(material.texDisplacement0, texCoords).r;
    
    while (layerDepth < depth) {
        texCoords += deltaTexCoords;
        depth = texture(material.texDisplacement0, texCoords).r;
        layerDepth += deltaLayerDepth;
    }
    vec2 lastTexCoords = texCoords - deltaTexCoords;
    float beforeDepth = texture(material.texDisplacement0, lastTexCoords).r - layerDepth + deltaLayerDepth;
    float afterDepth = depth - layerDepth;
    
    return mix(texCoords, lastTexCoords, afterDepth / (afterDepth - beforeDepth));
}

vec3 calculateLight(Light light, vec3 normalDir, vec3 viewDir, vec3 diffuseColor, vec3 specularColor) {    // Computes the color of a fragment with one light source. All positions/directions in world space.
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
    vec2 texCoords = mapParallax(fTexCoords, normalize(transpose(fTBNMtx) * (viewPosition - fPosition)));
    if (texCoords.x < 0.0 || texCoords.x > 1.0 || texCoords.y < 0.0 || texCoords.y > 1.0) {    // Discard fragment if it lies outside of coordinate range, may not work for all surfaces.
        //discard;
    }
    vec4 diffuseColor = texture(material.texDiffuse0, texCoords);
    vec4 specularColor = texture(material.texSpecular0, texCoords).rrra;
    vec3 normalDir = normalize(fTBNMtx * (texture(material.texNormal0, texCoords).rgb * 2.0 - 1.0));
    vec3 viewDir = normalize(viewPosition - fPosition);
    
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
