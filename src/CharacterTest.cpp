#include "CharacterTest.h"
#include "Shader.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>

void CharacterTest::init() {
    model_.loadFile("models/miku/miku4.fbx", &animations_);
    transform_.setPosition(glm::vec3(-2.0f, 0.0f, 2.0f));
    //transform_.setPosition(glm::vec3(0.0f, 12.0f, 0.0f));
    //transform_.setPitchYawRoll(glm::vec3(glm::pi<float>() / 2.0f, 0.0f, 0.0f));
    transform_.setScale(glm::vec3(1.0f));
    
    Animation nullAnimation("null", 30.0, 20.0);
    animations_.insert({"null", nullAnimation});
    
    Animation moveForwardAnimation("moveForward", 30.0, 20.0);
    moveForwardAnimation.channels_["Hips"].translationKeys.emplace_back(glm::vec3(0.0f, 0.0f, 0.0f), 0.0);
    moveForwardAnimation.channels_["Hips"].translationKeys.emplace_back(glm::vec3(0.0f, 1.0f, 0.0f), 30.0);
    moveForwardAnimation.channels_["Hips"].rotationKeys.emplace_back(glm::quat(0.657933f, 0.657933f, 0.259084f, 0.259084f), 0.0);
    moveForwardAnimation.channels_["Hips"].scalingKeys.emplace_back(glm::vec3(1.0f, 1.0f, 1.0f), 0.0);
    animations_.insert({"moveForward", moveForwardAnimation});
    
    assert(model_.boneOffsetMatrices_.size() <= ModelRigged::MAX_NUM_BONES);
    boneTransforms_.resize(model_.boneOffsetMatrices_.size(), glm::mat4(1.0f));
    
    const ModelRigged::Node* node = model_.findNode("Breast_R");
    if (node != nullptr) {
        dynamicBones_[node->boneIndex].maxDisplacement = 0.06f;
        dynamicBones_[node->boneIndex].maxDisplacementCOM = 1.9f;
        dynamicBones_[node->boneIndex].centerOfMassOffset = glm::vec3(0.0f, 1.0f, 0.0f);
        dynamicBones_[node->boneIndex].springMotion.computeMotionParams(1.0f, 0.2f, 0.5f);
        dynamicBones_[node->boneIndex].springMotionCOM.computeMotionParams(1.0f, 0.2f, 0.5f);
    }
    node = model_.findNode("Breast_L");
    if (node != nullptr) {
        dynamicBones_[node->boneIndex].maxDisplacement = 0.06f;
        dynamicBones_[node->boneIndex].maxDisplacementCOM = 1.9f;
        dynamicBones_[node->boneIndex].centerOfMassOffset = glm::vec3(0.0f, 1.0f, 0.0f);
        dynamicBones_[node->boneIndex].springMotion.computeMotionParams(1.0f, 0.2f, 0.5f);
        dynamicBones_[node->boneIndex].springMotionCOM.computeMotionParams(1.0f, 0.2f, 0.5f);
    }
}

void CharacterTest::update() {
    //model_.ragdoll(transform_.getTransform(), dynamicBones_, boneTransforms_);
    //model_.animate(animations_.at("Armature|Armature|mixamo.com|Layer0"), glfwGetTime(), boneTransforms_);
    //model_.animate(animations_.at("moveForward"), glfwGetTime(), boneTransforms_);
    //model_.animateWithDynamics(animations_.at("Armature|Armature|mixamo.com|Layer0"), glfwGetTime(), transform_.getTransform(), dynamicBones_, boneTransforms_);
    model_.animateWithDynamics(animations_.at("null"), glfwGetTime(), transform_.getTransform(), dynamicBones_, boneTransforms_);
}

void CharacterTest::draw(const Shader& shader, const glm::mat4& modelMtx) const {
    shader.setMat4Array("boneTransforms", static_cast<unsigned int>(boneTransforms_.size()), boneTransforms_.data());
    model_.draw(shader, transform_.getTransform());
}
