#ifndef COMMON_MATH_H_
#define COMMON_MATH_H_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

namespace CommonMath {    // may want to move more functions in here ##############################################################
    glm::quat findRotationBetweenVectors(glm::vec3 source, glm::vec3 destination);    // Computes the quaternion to rotate from source to destination direction vectors.
    glm::mat4 orientAt(glm::vec3 eye, glm::vec3 center, glm::vec3 up);    // Similar to glm::lookAt but instead of finding the view matrix, it computes an object transform.
}

#endif
