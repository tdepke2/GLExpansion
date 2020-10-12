#include "CharacterTest.h"
#include "Shader.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>

void CharacterTest::init() {
    model_.loadFile("models/miku/miku.fbx", &animations_);
    transform_.setPosition(glm::vec3(-2.0f, 0.0f, 2.0f));
    //transform_.setPosition(glm::vec3(0.0f, 12.0f, 0.0f));
    //transform_.setPitchYawRoll(glm::vec3(glm::pi<float>() / 2.0f, 0.0f, 0.0f));
    transform_.setScale(glm::vec3(1.0f));
    
    Animation::loadFile("models/miku/animHipHopDancing.fbx", &animations_);
    Animation::loadFile("models/miku/animJump.fbx", &animations_);
    Animation::loadFile("models/miku/animJumpingDown.fbx", &animations_);
    Animation::loadFile("models/miku/animShovedReactionWithSpin.fbx", &animations_);
    Animation::loadFile("models/miku/animWave.fbx", &animations_);
    
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
    
    ModelRigged::DynamicBone breastBone(0.03f, 1.9f, glm::vec3(0.0f, 1.0f, 0.0f), DampedSpringMotion(1.0f, 0.2f, 0.5f), DampedSpringMotion(1.0f, 0.2f, 0.5f));
    dynamicBones_[model_.findNode("Breast_R")->boneIndex] = breastBone;
    dynamicBones_[model_.findNode("Breast_L")->boneIndex] = breastBone;
    
    ModelRigged::DynamicBone hairBone(0.0f, 2.0f, glm::vec3(0.0f, 1.0f, 0.0f), DampedSpringMotion(), DampedSpringMotion(1.0f, 0.2f, 0.5f));
    dynamicBones_[model_.findNode("Antenna1")->boneIndex] = hairBone;
    dynamicBones_[model_.findNode("Antenna2")->boneIndex] = hairBone;
    dynamicBones_[model_.findNode("Sideburn1_R")->boneIndex] = hairBone;
    dynamicBones_[model_.findNode("Sideburn2_R")->boneIndex] = hairBone;
    dynamicBones_[model_.findNode("Sideburn1_L")->boneIndex] = hairBone;
    dynamicBones_[model_.findNode("Sideburn2_L")->boneIndex] = hairBone;
    dynamicBones_[model_.findNode("FrontHair1")->boneIndex] = hairBone;
    dynamicBones_[model_.findNode("FrontHair1_2")->boneIndex] = hairBone;
    dynamicBones_[model_.findNode("FrontHair2")->boneIndex] = hairBone;
    dynamicBones_[model_.findNode("FrontHair2_2")->boneIndex] = hairBone;
    dynamicBones_[model_.findNode("FrontHair3")->boneIndex] = hairBone;
    dynamicBones_[model_.findNode("FrontHair3_2")->boneIndex] = hairBone;
    
    dynamicBones_[model_.findNode("Hair1_R")->boneIndex] = hairBone;
    dynamicBones_[model_.findNode("Hair2_R")->boneIndex] = hairBone;
    dynamicBones_[model_.findNode("Hair3_R")->boneIndex] = hairBone;
    dynamicBones_[model_.findNode("Hair4_R")->boneIndex] = hairBone;
    dynamicBones_[model_.findNode("Hair5_R")->boneIndex] = hairBone;
    dynamicBones_[model_.findNode("Hair6_R")->boneIndex] = hairBone;
    dynamicBones_[model_.findNode("Hair7_R")->boneIndex] = hairBone;
    dynamicBones_[model_.findNode("Hair8_R")->boneIndex] = hairBone;
    
    dynamicBones_[model_.findNode("Hair1_L")->boneIndex] = hairBone;
    dynamicBones_[model_.findNode("Hair2_L")->boneIndex] = hairBone;
    dynamicBones_[model_.findNode("Hair3_L")->boneIndex] = hairBone;
    dynamicBones_[model_.findNode("Hair4_L")->boneIndex] = hairBone;
    dynamicBones_[model_.findNode("Hair5_L")->boneIndex] = hairBone;
    dynamicBones_[model_.findNode("Hair6_L")->boneIndex] = hairBone;
    dynamicBones_[model_.findNode("Hair7_L")->boneIndex] = hairBone;
    dynamicBones_[model_.findNode("Hair8_L")->boneIndex] = hairBone;
    
    ModelRigged::DynamicBone ribbonBone(0.0f, 2.0f, glm::vec3(0.0f, 1.0f, 0.0f), DampedSpringMotion(), DampedSpringMotion(1.0f, 0.1f, 0.7f));
    ModelRigged::DynamicBone ribbonBoneReversed = ribbonBone;
    ribbonBoneReversed.centerOfMassOffset = glm::vec3(0.0f, -1.0f, 0.0f);
    dynamicBones_[model_.findNode("HairRibbonTip_R")->boneIndex] = ribbonBoneReversed;
    dynamicBones_[model_.findNode("HairRibbonTip+_R")->boneIndex] = ribbonBoneReversed;
    dynamicBones_[model_.findNode("RibbonBack1_R")->boneIndex] = ribbonBone;
    dynamicBones_[model_.findNode("RibbonBack2_R")->boneIndex] = ribbonBone;
    dynamicBones_[model_.findNode("RibbonLeg1_R")->boneIndex] = ribbonBone;
    dynamicBones_[model_.findNode("RibbonLeg2_R")->boneIndex] = ribbonBone;
    
    dynamicBones_[model_.findNode("HairRibbonTip_L")->boneIndex] = ribbonBoneReversed;
    dynamicBones_[model_.findNode("HairRibbonTip+_L")->boneIndex] = ribbonBoneReversed;
    dynamicBones_[model_.findNode("RibbonBack1_L")->boneIndex] = ribbonBone;
    dynamicBones_[model_.findNode("RibbonBack2_L")->boneIndex] = ribbonBone;
    dynamicBones_[model_.findNode("RibbonLeg1_L")->boneIndex] = ribbonBone;
    dynamicBones_[model_.findNode("RibbonLeg2_L")->boneIndex] = ribbonBone;
}

void CharacterTest::update() {
    //model_.ragdoll(transform_.getTransform(), dynamicBones_, boneTransforms_);
    //model_.animate(animations_.at("Armature|Armature|mixamo.com|Layer0"), glfwGetTime(), boneTransforms_);
    //model_.animate(animations_.at("moveForward"), glfwGetTime(), boneTransforms_);
    model_.animateWithDynamics(animations_.at("Armature|wave"), glfwGetTime(), transform_.getTransform(), dynamicBones_, boneTransforms_);
    //model_.animateWithDynamics(animations_.at("null"), glfwGetTime(), transform_.getTransform(), dynamicBones_, boneTransforms_);
}

void CharacterTest::draw(const Shader& shader, const glm::mat4& modelMtx) const {
    shader.setMat4Array("boneTransforms", static_cast<unsigned int>(boneTransforms_.size()), boneTransforms_.data());
    model_.draw(shader, transform_.getTransform());
}
