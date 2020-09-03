#version 330 core

uniform sampler2D texPosition;
uniform sampler2D texNormal;
uniform sampler2D texAlbedoSpec;
uniform vec2 renderSize;
uniform vec3 color;
uniform vec3 phongVals;
uniform vec3 attenuation;

in vec3 fLightPositionVS;

out vec4 fragColor;

vec3 calculateLight(vec3 position, vec3 normal, vec3 diffuseColor, float specularColor, float ambientOcclusion, vec3 viewDir) {    // Computes the color of a fragment with one light source. All positions/directions in view space.
    vec3 lightDir = normalize(fLightPositionVS - position);
    float distanceFragToLight = length(fLightPositionVS - position);
    float lightScalar = 1.0 / (attenuation.x + attenuation.y * distanceFragToLight + attenuation.z * distanceFragToLight * distanceFragToLight);
    
    vec3 ambient = color * phongVals.x * diffuseColor * lightScalar * ambientOcclusion;
    
    float diffuseScalar = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = color * phongVals.y * diffuseScalar * diffuseColor * lightScalar;
    
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float specularScalar = (diffuseScalar == 0.0 ? 0.0 : pow(max(dot(normal, halfwayDir), 0.0), 64.0));    // Blinn-Phong model.
    vec3 specular = color * phongVals.z * specularScalar * specularColor * lightScalar;
    
    return ambient + diffuse + specular;
}

void main() {
    /*vec3 position = texture(texPosition, fTexCoords).rgb;
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
    }*/
    
    vec2 texCoords = gl_FragCoord.xy / renderSize;
    vec3 position = texture(texPosition, texCoords).rgb;
    vec3 normal = texture(texNormal, texCoords).rgb;
    vec3 diffuseColor = texture(texAlbedoSpec, texCoords).rgb;
    float specularColor = texture(texAlbedoSpec, texCoords).a;
    float ambientOcclusion = 1.0;
    fragColor = vec4(calculateLight(position, normal, diffuseColor, specularColor, ambientOcclusion, normalize(-position)), 1.0);
}
