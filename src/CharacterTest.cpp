#include "CharacterTest.h"
#include "Shader.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>

void CharacterTest::init() {
    model_.loadFile("models/miku/miku.fbx");
    transform_.setPosition(glm::vec3(-20.0f, 0.0f, -12.0f));
    //transform_.setPosition(glm::vec3(0.0f, 12.0f, 0.0f));
    //transform_.setPitchYawRoll(glm::vec3(glm::pi<float>() / 2.0f, 0.0f, 0.0f));
    transform_.setScale(glm::vec3(1.0f));
    
    assert(model_.boneOffsetMatrices_.size() <= ModelRigged::MAX_NUM_BONES);
    boneTransforms_.resize(model_.boneOffsetMatrices_.size(), glm::mat4(1.0f));
    
    activeForces_.push_back(glm::vec3(0.0f, -0.01f, 0.0f));
    lastBoneTransform_ = glm::mat4(1.0f);
    
    const ModelRigged::Node* node = model_.findNode("Breast_R");
    if (node != nullptr) {
        //dynamicBones_[node->boneIndex].angularAcc = glm::quat(glm::vec3(0.0001f, 0.0f, 0.0f));
        dynamicBones_[node->boneIndex].linearVel = glm::vec3(0.0f, 0.0f, 0.05f);
        dynamicBones_[node->boneIndex].springMotion.computeMotionParams(1.0f, 0.1f, 0.1f);
        //dynamicBones_[node->boneIndex].angularAcc = glm::quat(glm::vec3(0.0001f, 0.0f, 0.0f));
        //dynamicBones_[node->boneIndex].angularVel = glm::quat(glm::vec3(0.01f, 0.0f, 0.0f));
    }
}

void CharacterTest::update() {
    /*for (glm::mat4& m : boneTransforms_) {
        m = glm::rotate(glm::mat4(1.0f), static_cast<float>(sin(glfwGetTime() * 0.3)), glm::vec3(0.0f, 1.0f, 0.0f));
        m = glm::rotate(m, static_cast<float>(sin(glfwGetTime() * 0.21)), glm::vec3(0.0f, 0.0f, 1.0f));
        m = glm::rotate(m, static_cast<float>(sin(glfwGetTime() * 0.16)), glm::vec3(1.0f, 0.0f, 0.0f));
    }
    
    const ModelRigged::Node* node = model_.findNode("Breast_R");
    if (node != nullptr) {
        glm::vec3 heading(0.0f, 0.0f, 1.0f);
        glm::mat4 boneTransformMS = boneTransforms_[node->boneIndex] * glm::inverse(model_.boneOffsetMatrices_[node->boneIndex]);
        glm::mat4 boneTransformWS = transform_.getTransform() * glm::inverse(model_.getGlobalInverseMtx()) * boneTransformMS;
        
        //glm::vec3 position = glm::vec3(boneTransformMS * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        //glm::vec3 direction = glm::normalize(glm::vec3(boneTransformMS * glm::vec4(heading, 0.0f)));
        
        // previous bone:
        // velocity, acceleration, angular vel, torque.
        
        glm::vec3 boneDir = glm::normalize(glm::mat3(boneTransformWS) * heading);
        glm::vec3 newBoneDir = glm::vec3(0.0f, -1.0f, 0.0f);//glm::mat3(glm::rotate(glm::mat4(1.0f), 1.0f, glm::vec3(1.0f, 0.0f, 0.0f))) * boneDir;
        glm::mat4 r = glm::mat4_cast(findRotationBetweenVectors(boneDir, newBoneDir));
        //glm::mat4 r = glm::rotate(glm::mat4(1.0f), );
        
        boneTransformWS = glm::rotate(boneTransformWS, static_cast<float>(sin(glfwGetTime())), glm::vec3(1.0f, 0.0f, 0.0f));
        //boneTransformWS = glm::translate(glm::mat4(1.0f), glm::vec3(boneTransformWS * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)));
        //boneTransformWS = boneTransformWS * r;
        
        lastBoneTransform_ = boneTransformWS;
        //boneTransforms_[node->boneIndex] = glm::translate(glm::rotate(glm::translate(glm::mat4(1.0f), position), static_cast<float>(sin(glfwGetTime())), glm::vec3(1.0f, 0.0f, 0.0f)), -position) * boneTransforms_[node->boneIndex];
        //boneTransforms_[node->boneIndex] = glm::rotate(boneTransformMS, static_cast<float>(sin(glfwGetTime())), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::inverse(boneTransformMS) * boneTransforms_[node->boneIndex];    // From right to left go from model space, to bone space, apply transform, and back to model space.
        boneTransforms_[node->boneIndex] = model_.getGlobalInverseMtx() * glm::inverse(transform_.getTransform()) * boneTransformWS * model_.boneOffsetMatrices_[node->boneIndex];
    }*/
    
    model_.ragdoll(dynamicBones_, boneTransforms_);
}

void CharacterTest::draw(const Shader& shader, const glm::mat4& modelMtx) const {
    shader.setMat4Array("boneTransforms", static_cast<unsigned int>(boneTransforms_.size()), boneTransforms_.data());
    model_.draw(shader, transform_.getTransform());
}
