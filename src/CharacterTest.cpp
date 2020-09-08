#include "CharacterTest.h"
#include "Shader.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

void CharacterTest::init() {
    model_.loadFile("models/miku/miku.fbx");
    //transform_.setPosition(glm::vec3(-20.0f, 0.0f, -12.0f));
    transform_.setPosition(glm::vec3(0.0f, 12.0f, 0.0f));
    transform_.setScale(glm::vec3(1.0f));
}

void CharacterTest::draw(const Shader& shader, const glm::mat4& modelMtx) const {
    model_.draw(shader, transform_.getTransform());
}
