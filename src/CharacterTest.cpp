#include "CharacterTest.h"
#include "Shader.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

void CharacterTest::init() {
    ModelAbstract::VERBOSE_OUTPUT_ = true;
    model_.loadFile("models/miku/miku.fbx");
    ModelAbstract::VERBOSE_OUTPUT_ = false;
    //transform_.setPosition(glm::vec3(-20.0f, 0.0f, -12.0f));
    transform_.setPosition(glm::vec3(0.0f, 12.0f, 0.0f));
    transform_.setScale(glm::vec3(1.0f));
    
    assert(model_.boneOffsetMatrices_.size() <= ModelRigged::MAX_NUM_BONES);
    boneTransforms_.resize(model_.boneOffsetMatrices_.size(), glm::mat4(1.0f));
}

void CharacterTest::update() {
    //model_.animate(0, glfwGetTime(), boneTransforms_);
}

void CharacterTest::draw(const Shader& shader, const glm::mat4& modelMtx) const {
    shader.setMat4Array("boneTransforms", static_cast<unsigned int>(boneTransforms_.size()), boneTransforms_.data());
    model_.draw(shader, transform_.getTransform());
}
