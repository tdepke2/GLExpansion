#include "Camera.h"

Camera::Camera(const glm::vec3& position, const glm::vec3& worldUp, float yaw, float pitch) {
    position_ = position;
    worldUp_ = worldUp;
    yaw_ = yaw;
    pitch_ = pitch;
    moveSpeed_ = 2.5f;
    mouseSensistivity_ = 0.1f;
    fov_ = 90.0f;
    updateRotation();
}

glm::mat4 Camera::getViewMatrix() {
    return glm::lookAt(position_, position_ + front_, up_);
}

void Camera::processKeyboard(const glm::vec3& direction, float deltaTime) {
    float velocity = moveSpeed_ * deltaTime;
    position_ += direction.x * right_ * velocity;
    position_ += direction.y * up_ * velocity;
    position_ += direction.z * -front_ * velocity;
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

void Camera::updateRotation() {
    front_.x = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
    front_.y = sin(glm::radians(pitch_));
    front_.z = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));
    right_ = glm::normalize(glm::cross(front_, worldUp_));
    up_ = glm::normalize(glm::cross(right_, front_));
}
