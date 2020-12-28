#version 330 core

const float PI = 3.14159265359;

uniform samplerCube environmentCubemap;
uniform float roughness;

in vec3 fPosition;

out vec4 fragColor;

float radicalInverseVanDerCorpus(uint bits) {    // Efficient VDC calculation http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10;    // 0x100000000
}

vec2 hammersley(uint i, uint N) {    // Compute the low-discrepancy sample i out of N samples using the Hammersley sequence.
    return vec2(float(i) / float(N), radicalInverseVanDerCorpus(i));
}

float vanDerCorpus(uint n, uint base) {
    float invBase = 1.0 / float(base);
    float denom = 1.0;
    float result = 0.0;
    
    for (uint i = 0u; i < 32u; ++i) {
        if (n > 0u) {
            denom = mod(float(n), 2.0);
            result += denom * invBase;
            invBase = invBase / 2.0;
            n = uint(float(n) / 2.0);
        }
    }
    
    return result;
}

vec2 hammersleyNoBitOps(uint i, uint N) {    // Alternative Hammersley function that does not use bitwise operations for architectures that don't support them.
    return vec2(float(i) / float(N), vanDerCorpus(i, 2u));
}

vec3 importanceSampleGGX(vec2 Xi, vec3 N, float alpha) {
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (alpha * alpha - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    
    vec3 tangentSample = vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);    // Spherical to cartesian.
    
    vec3 upVec = (abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0));    // Find tangent-space vectors.
    vec3 tangentVec = normalize(cross(upVec, N));
    vec3 bitangentVec = cross(N, tangentVec);
    
    return normalize(vec3(tangentSample.x * tangentVec + tangentSample.y * bitangentVec + tangentSample.z * N));    // Sample in world-space.
}

float distributionGGX(float dotNH, float alpha) {    // Normal distribution function used to approximate surface microfacets (Trowbridge-Reitz GGX).
    float alpha2 = alpha * alpha;
    float x = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
    return alpha2 / (PI * x * x);
}

void main() {
    vec3 N = normalize(fPosition);    // Assume view and reflection direction equals the normal.
    float alpha = roughness * roughness;
    
    const uint NUM_SAMPLES = 1024u;
    float totalWeight = 0.0;
    vec3 prefilteredColor = vec3(0.0);
    for (uint i = 0u; i < NUM_SAMPLES; ++i) {
        vec2 Xi = hammersley(i, NUM_SAMPLES);    // Generate sample vector that's biased towards the normal.
        vec3 H = importanceSampleGGX(Xi, N, alpha);
        vec3 L = normalize(2.0 * dot(N, H) * H - N);
        
        float dotNL = dot(N, L);
        if (dotNL > 0.0) {
            float NDF = distributionGGX(max(dot(N, H), 0.0), alpha);    // Sample environmentCubemap mip level based on roughness/pdf.
            float dotNH = max(dot(N, H), 0.0);
            float pdf = NDF * dotNH / (4.0 * dotNH) + 0.0001;
            
            float resolution = textureSize(environmentCubemap, 0).x;
            float saTexel = 4.0 * PI / (6.0 * resolution * resolution);
            float saSample = 1.0 / (float(NUM_SAMPLES) * pdf + 0.0001);
            float mipLevel = (roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel));
            
            prefilteredColor += textureLod(environmentCubemap, L, mipLevel).rgb * dotNL;
            totalWeight += dotNL;
        }
    }
    
    prefilteredColor = prefilteredColor / totalWeight;
    
    fragColor = vec4(prefilteredColor, 1.0);
}
