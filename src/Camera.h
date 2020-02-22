#ifndef _CAMERA_H
#define _CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

class Camera {
    public:
    glm::vec3 position, front, up, right, worldUp;
    float yaw, pitch, moveSpeed, mouseSensistivity, fov;
    
    Camera(const glm::vec3& position = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3& worldUp = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = -90.0f, float pitch = 0.0f);
    glm::mat4 getViewMatrix();
    void processKeyboard(const glm::vec3& direction, float deltaTime);
    void processMouseMove(float xoffset, float yoffset, bool constrainPitch = true);
    void processMouseScroll(float xoffset, float yoffset);
    void updateRotation();
};

#endif
