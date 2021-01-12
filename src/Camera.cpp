#include "Camera.h"
#include "SceneNode.h"

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(getSceneNode()->getPosition(), getSceneNode()->getPosition() + front_, up_);
}

void Camera::processKeyboard(const glm::vec3& direction) {
    getSceneNode()->move(direction.x * right_ * moveSpeed_);
    getSceneNode()->move(direction.y * up_ * moveSpeed_);
    getSceneNode()->move(direction.z * -front_ * moveSpeed_);
}

void Camera::processMouseMove(float xoffset, float yoffset, bool constrainPitch) {
    xoffset *= mouseSensistivity_;
    yoffset *= mouseSensistivity_;
    
    yaw_ = fmod(yaw_ + xoffset, 360.0f);
    if (constrainPitch) {
        pitch_ += yoffset;
        if (pitch_ > 89.9f) {
            pitch_ = 89.9f;
        } else if (pitch_ < -89.9f) {
            pitch_ = -89.9f;
        }
    } else {
        pitch_ = fmod(pitch_ + yoffset, 360.0f);
    }
    
    updateRotation();
}

void Camera::processMouseScroll(float xoffset, float yoffset) {
    fov_ -= yoffset * 2;
    if (fov_ > 90.0f) {
        fov_ = 90.0f;
    } else if (fov_ < 1.0f) {
        fov_ = 1.0f;
    }
}

void Camera::draw() const {}

Camera::Camera(const string& name, const glm::vec3& worldUp, float yaw, float pitch) :
    SceneObject(name),
    worldUp_(worldUp),
    yaw_(yaw),
    pitch_(pitch),
    moveSpeed_(2.5f * 0.016f),
    mouseSensistivity_(0.1f),
    fov_(90.0f) {
    
    updateRotation();
}

void Camera::updateRotation() {
    front_.x = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
    front_.y = sin(glm::radians(pitch_));
    front_.z = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));
    right_ = glm::normalize(glm::cross(front_, worldUp_));
    up_ = glm::normalize(glm::cross(right_, front_));
}
