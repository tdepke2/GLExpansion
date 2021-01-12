#ifndef CAMERA_H_
#define CAMERA_H_

#include "SceneObject.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

using namespace std;

class Camera : public SceneObject {
    public:
    glm::vec3 front_, up_, right_, worldUp_;
    float yaw_, pitch_, moveSpeed_, mouseSensistivity_, fov_;
    
    glm::mat4 getViewMatrix() const;
    void processKeyboard(const glm::vec3& direction);
    void processMouseMove(float xoffset, float yoffset, bool constrainPitch = true);
    void processMouseScroll(float xoffset, float yoffset);
    
    protected:
    void draw() const;
    
    private:
    Camera(const string& name, const glm::vec3& worldUp = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = -90.0f, float pitch = 0.0f);
    void updateRotation();
    
    friend class Scene;
};

#endif
