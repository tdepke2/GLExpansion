#ifndef CAMERA_H_
#define CAMERA_H_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

class Camera {
    public:
    glm::vec3 position_, front_, up_, right_, worldUp_;
    float yaw_, pitch_, moveSpeed_, mouseSensistivity_, fov_;
    
    Camera(const glm::vec3& position = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3& worldUp = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = -90.0f, float pitch = 0.0f);
    glm::mat4 getViewMatrix() const;
    void processKeyboard(const glm::vec3& direction);
    void processMouseMove(float xoffset, float yoffset, bool constrainPitch = true);
    void processMouseScroll(float xoffset, float yoffset);
    void updateRotation();
};

#endif
