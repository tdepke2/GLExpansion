#version 330 core

const float PI = 3.14159265359;

in vec2 fTexCoords;

out vec2 fragColor;

float radicalInverseVanDerCorpus(uint bits) {
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

vec3 importanceSampleGGX(vec2 Xi, vec3 N, float a) {
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    
    vec3 H = vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);    // Spherical to cartesian.
    
    vec3 upVec = (abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0));    // Find tangent-space vectors.
    vec3 tangentVec = normalize(cross(upVec, N));
    vec3 bitangentVec = cross(N, tangentVec);
    
    return normalize(vec3(tangentVec * H.x + bitangentVec * H.y + N * H.z));    // Sample in world-space.
}

float geometrySchlickGGX(float dotNV, float a) {
    float k = a / 2.0;    // We use a different value for k in IBL than with point lights.
    return dotNV / (dotNV * (1.0 - k) + k);    // may want to confirm that k = a/2 and not a^2/2 ########################################
}

float geometrySmith(float dotNV, float dotNL, float a) {
    float ggx2 = geometrySchlickGGX(dotNV, a);
    float ggx1 = geometrySchlickGGX(dotNL, a);
    return ggx1 * ggx2;
}

void main() {
    float dotNV = max(fTexCoords.x, 0.0);
    float alpha = fTexCoords.y * fTexCoords.y;
    vec3 N = vec3(0.0, 0.0, 1.0);
    vec3 V = vec3(sqrt(1.0 - dotNV * dotNV), 0.0, dotNV);
    
    float scale = 0.0;
    float bias = 0.0;
    
    const uint NUM_SAMPLES = 1024u;
    for (uint i = 0u; i < NUM_SAMPLES; ++i) {
        vec2 Xi = hammersley(i, NUM_SAMPLES);
        vec3 H = importanceSampleGGX(Xi, N, alpha);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);
        
        float dotNL = max(L.z, 0.0);
        float dotNH = max(H.z, 0.0);
        float dotVH = max(dot(V, H), 0.0);
        
        if (dotNL > 0.0) {
            float G = geometrySmith(dotNV, dotNL, alpha);
            float G_Vis = (G * dotVH) / (dotNH * dotNV);
            float Fc = pow(1.0 - dotVH, 5.0);
            
            scale += (1.0 - Fc) * G_Vis;
            bias += Fc * G_Vis;
        }
    }
    
    fragColor = vec2(scale, bias) / float(NUM_SAMPLES);
}
