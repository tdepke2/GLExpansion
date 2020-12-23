#version 330 core

const float PI = 3.14159265359;

uniform samplerCube environmentCubemap;

in vec3 fPosition;

out vec4 fragColor;

void main() {
    vec3 normal = normalize(fPosition);    // Sample direction is the orientation of the hemisphere.
    vec3 irradiance = vec3(0.0);
    
    vec3 upVec = vec3(0.0, 1.0, 0.0);
    vec3 rightVec = normalize(cross(upVec, normal));
    upVec = cross(normal, rightVec);
    
    const float SAMPLE_DELTA = 0.025;
    float numSamples = 0.0;
    for (float phi = 0.0; phi < 2.0 * PI; phi += SAMPLE_DELTA) {
        for (float theta = 0.0; theta < 0.5 * PI; theta += SAMPLE_DELTA) {
            vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));    // Spherical to cartesian (in tangent space).
            vec3 sampleVec = tangentSample.x * rightVec + tangentSample.y * upVec + tangentSample.z * normal;    // Tangent space to world space.
            
            irradiance += texture(environmentCubemap, sampleVec).rgb * cos(theta) * sin(theta);
            ++numSamples;
        }
    }
    
    irradiance = PI * irradiance / numSamples;
    
    irradiance = texture(environmentCubemap, fPosition).rgb;    // temp for testing only #######################################
    
    fragColor = vec4(irradiance, 1.0);
}
