#include "Camera.h"
#include <iostream>

Camera::Camera(const glm::vec3& position, const glm::vec3& worldUp, float yaw, float pitch) {
    this->position = position;
    this->worldUp = worldUp;
    this->yaw = yaw;
    this->pitch = pitch;
    moveSpeed = 2.5f;
    mouseSensistivity = 0.1f;
    fov = 90.0f;
    updateRotation();
}

glm::mat4 Camera::getViewMatrix() {
    return glm::lookAt(position, position + front, up);
}

void Camera::processKeyboard(const glm::vec3& direction, float deltaTime) {
    float velocity = moveSpeed * deltaTime;
    position += direction.z * -front * velocity;
    position += direction.x * right * velocity;
}

void Camera::processMouseMove(float xoffset, float yoffset, bool constrainPitch) {
    xoffset *= mouseSensistivity;
    yoffset *= mouseSensistivity;
    
    yaw = fmod(yaw + xoffset, 360.0f);
    if (constrainPitch) {
        pitch += yoffset;
        if (pitch > 89.9f) {
            pitch = 89.9f;
        } else if (pitch < -89.9f) {
            pitch = -89.9f;
        }
    } else {
        pitch = fmod(pitch + yoffset, 360.0f);
    }
    
    updateRotation();
}

void Camera::processMouseScroll(float xoffset, float yoffset) {
    fov -= yoffset * 2;
    if (fov > 90.0f) {
        fov = 90.0f;
    } else if (fov < 1.0f) {
        fov = 1.0f;
    }
}

void Camera::updateRotation() {
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}
