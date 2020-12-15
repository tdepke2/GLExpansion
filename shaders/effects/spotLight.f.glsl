#version 330 core

uniform sampler2D texPosition;
uniform sampler2D texNormal;
uniform sampler2D texAlbedoSpec;
uniform sampler2D texSSAO;
uniform bool applySSAO;
uniform vec2 renderSize;
uniform vec3 color;
uniform vec3 phongVals;
uniform vec3 attenuation;
uniform vec2 cutOff;

in vec3 fLightPositionVS;
in vec3 fLightDirectionVS;

out vec4 fragColor;

vec3 calculateLight(vec3 position, vec3 normal, vec3 albedoColor, float specularColor, float ambientOcclusion, vec3 viewDir) {    // Computes the color of a fragment with one light source. All positions/directions in view space.
    vec3 lightDir = normalize(fLightPositionVS - position);
    float distanceFragToLight = length(fLightPositionVS - position);
    float lightScalar = 1.0 / (attenuation.x + attenuation.y * distanceFragToLight + attenuation.z * distanceFragToLight * distanceFragToLight);
    
    vec3 ambient = color * phongVals.x * albedoColor * lightScalar * ambientOcclusion;
    
    float theta = dot(lightDir, normalize(-fLightDirectionVS));
    float intensity = clamp((theta - cutOff.y) / (cutOff.x - cutOff.y), 0.0, 1.0);
    lightScalar *= intensity;
    
    float diffuseScalar = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = color * phongVals.y * diffuseScalar * albedoColor * lightScalar;
    
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float specularScalar = (diffuseScalar == 0.0 ? 0.0 : pow(max(dot(normal, halfwayDir), 0.0), 64.0));    // Blinn-Phong model.
    vec3 specular = color * phongVals.z * specularScalar * specularColor * lightScalar;
    
    return ambient + diffuse + specular;
}

void main() {
    vec2 texCoords = gl_FragCoord.xy / renderSize;
    vec3 position = texture(texPosition, texCoords).rgb;
    vec3 normal = texture(texNormal, texCoords).rgb;
    vec3 albedoColor = texture(texAlbedoSpec, texCoords).rgb;
    float specularColor = texture(texAlbedoSpec, texCoords).a;
    float ambientOcclusion = (applySSAO ? texture(texSSAO, texCoords).r : 1.0);
    vec3 viewDir = normalize(-position);
    
    fragColor = vec4(calculateLight(position, normal, albedoColor, specularColor, ambientOcclusion, viewDir), 1.0);
}
