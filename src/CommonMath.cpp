#include "CommonMath.h"
#include <cmath>

glm::quat CommonMath::findRotationBetweenVectors(glm::vec3 source, glm::vec3 destination) {
    source = glm::normalize(source);
    destination = glm::normalize(destination);
    float cosTheta = glm::dot(source, destination);
    
    if (cosTheta < -1.0f + 0.001f) {    // Special case where vectors are in opposite direction.
        glm::vec3 axis = glm::cross(source, glm::vec3(1.0f, 0.0f, 0.0f));
        if (glm::length(axis) < 0.01f) {
            axis = glm::cross(source, glm::vec3(0.0f, 1.0f, 0.0f));
        }
        return glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::pi<float>(), axis);
    }
    
    glm::vec3 axis = glm::cross(source, destination);
    float s = sqrt((1.0f + cosTheta) * 2.0f);
    return glm::quat(s / 2.0f, axis.x / s, axis.y / s, axis.z / s);
}

glm::mat4 CommonMath::orientAt(glm::vec3 eye, glm::vec3 center, glm::vec3 up) {
    glm::vec3 f = glm::normalize(eye - center);    // Front.
    glm::vec3 r = glm::normalize(glm::cross(f, up));    // Right.
    glm::vec3 u = glm::cross(r, f);    // Up.
    
    glm::mat4 result(1.0f);
    result[0][0] = r.x;
    result[0][1] = r.y;
    result[0][2] = r.z;
    result[1][0] = u.x;
    result[1][1] = u.y;
    result[1][2] = u.z;
    result[2][0] = -f.x;
    result[2][1] = -f.y;
    result[2][2] = -f.z;
    result[3][0] = eye.x;
    result[3][1] = eye.y;
    result[3][2] = eye.z;
    
    return result;
}
