#version 330 core

const uint NUM_SAMPLES = 32u;
const float RADIUS = 1.0;
const float BIAS = 0.05;

layout (std140) uniform ViewProjectionMtx {
    uniform mat4 viewMtx;
    uniform mat4 projectionMtx;
};

uniform sampler2D texPosition;
uniform sampler2D texNormal;
uniform sampler2D texNoise;
uniform vec3 samples[NUM_SAMPLES];
uniform vec2 noiseScale;

in vec2 fTexCoords;

out float fragColor;

void main() {
    vec3 position = texture(texPosition, fTexCoords).rgb;
    vec3 normal = texture(texNormal, fTexCoords).rgb;
    vec3 noiseVec = texture(texNoise, fTexCoords * noiseScale).rgb;
    
    vec3 tangent = normalize(noiseVec - normal * dot(noiseVec, normal));    // Apply Gram-Schmidt process to get a change-of-basis matrix to convert to view space.
    vec3 bitangent = cross(normal, tangent);
    mat3 TBNMtx = mat3(tangent, bitangent, normal);
    
    float occlusion = 0.0;
    for (uint i = 0u; i < NUM_SAMPLES; ++i) {
        vec3 sample = position + TBNMtx * samples[i] * RADIUS;    // Get sample position in view space.
        
        vec4 sampleNDC = projectionMtx * vec4(sample, 1.0);    // Project sample to normalized device coords.
        sampleNDC.xyz /= sampleNDC.w;
        sampleNDC.xyz = sampleNDC.xyz * 0.5 + 0.5;
        
        float sampleDepth = texture(texPosition, sampleNDC.xy).z;
        if (sampleDepth != 0.0) {    // Make sure the sample is not the background.
            float rangeCheck = smoothstep(0.0, 1.0, RADIUS / abs(position.z - sampleDepth));
            occlusion += (sampleDepth >= sample.z + BIAS ? 1.0 : 0.0) * rangeCheck;
        }
    }
    fragColor = pow(1.0 - (occlusion / NUM_SAMPLES), 8.0);
}
