#include "World.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <random>

float World::calcLightRadius(const glm::vec3& color, const glm::vec3& attenuation) {
    float intensityMax = max(max(color.r, color.g), color.b);    // Equation derived from https://learnopengl.com/Advanced-Lighting/Deferred-Shading
    return (-attenuation.y + sqrt(attenuation.y * attenuation.y - 4.0f * attenuation.z * (attenuation.x - (256.0f / 5.0f) * intensityMax))) / (2.0f * attenuation.z);
}

World::World() :
    flashlightOn_(false),
    sunlightOn_(true),
    lampsOn_(false),
    sunT_(-2.0f),
    sunSpeed_(0.0f) {
    
    lightCube_.generateCube(0.05f);
    lightSphere_.generateSphere(1.0f, 16, 8);
    lightCone_.generateCylinder(0.0f, 1.0f, 1.0f, 16, 1, true);
    cube1_.generateCube();
    
    //sceneTest_.loadFile("models/boot_camp/boot_camp.obj");
    sceneTestTransform_.setScale(glm::vec3(0.025f, 0.025f, 0.025f));
    sceneTestTransform_.setPitchYawRoll(glm::vec3(-glm::pi<float>() / 2.0f, 0.0f, 0.0f));
    
    modelTest_.loadFile("models/bob_lamp_update/bob_lamp_update.md5mesh");
    //modelTest_.loadFile("models/hellknight/hellknight.md5mesh");
    //modelTest_.loadFile("models/spaceship/Intergalactic Spaceship_Blender_2.8_Packed textures.dae");
    modelTestTransform_.setScale(glm::vec3(0.3f));
    modelTestTransform_.setPosition(glm::vec3(0.0f, 0.0f, 2.0f));
    
    assert(modelTest_.boneOffsetMatrices_.size() <= ModelRigged::MAX_NUM_BONES);
    modelTestBoneTransforms_.resize(modelTest_.boneOffsetMatrices_.size(), glm::mat4(1.0f));
    
    characterTest_.init();
    
    sunLight_.color = glm::vec3(1.0f, 1.0f, 1.0f);
    sunLight_.phongVals =  glm::vec3(0.05f, 0.4f, 0.5f);
    
    pointLights_.resize(4);
    pointLights_[0].color = glm::vec3(5.0f, 5.0f, 5.0f);
    pointLights_[0].phongVals = glm::vec3(0.05f, 0.8f, 1.0f);
    pointLights_[0].attenuation = glm::vec3(1.0f, 0.7f, 1.8f);
    pointLights_[0].modelMtx = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.7f, 0.2f, 2.0f)), glm::vec3(calcLightRadius(pointLights_[0].color, pointLights_[0].attenuation)));
    
    pointLights_[1].color = glm::vec3(10.0f, 0.0f, 0.0f);
    pointLights_[1].phongVals = glm::vec3(0.05f, 0.8f, 1.0f);
    pointLights_[1].attenuation = glm::vec3(1.0f, 0.7f, 1.8f);
    pointLights_[1].modelMtx = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(2.3f, -3.3f, -4.0f)), glm::vec3(calcLightRadius(pointLights_[1].color, pointLights_[1].attenuation)));
    
    pointLights_[2].color = glm::vec3(0.0f, 0.0f, 15.0f);
    pointLights_[2].phongVals = glm::vec3(0.05f, 0.8f, 1.0f);
    pointLights_[2].attenuation = glm::vec3(1.0f, 0.7f, 1.8f);
    pointLights_[2].modelMtx = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-4.0f, 2.0f, -12.0f)), glm::vec3(calcLightRadius(pointLights_[2].color, pointLights_[2].attenuation)));
    
    pointLights_[3].color = glm::vec3(0.0f, 5.0f, 0.0f);
    pointLights_[3].phongVals = glm::vec3(0.05f, 0.8f, 1.0f);
    pointLights_[3].attenuation = glm::vec3(1.0f, 0.7f, 1.8f);
    pointLights_[3].modelMtx = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.1f, -3.0f)), glm::vec3(calcLightRadius(pointLights_[3].color, pointLights_[3].attenuation)));
    
    mt19937 rng;
    rng.seed(1);
    uniform_real_distribution<float> randRange(-40.0f, 40.0f);
    uniform_real_distribution<float> randColor(0.0f, 2.0f);
    
    pointLights_.resize(Renderer::NUM_LIGHTS - 2);
    for (size_t i = 4; i < pointLights_.size(); ++i) {
        glm::vec3 position(randRange(rng), randRange(rng), randRange(rng));
        pointLights_[i].color = glm::vec3(randColor(rng), randColor(rng), randColor(rng));
        pointLights_[i].phongVals = glm::vec3(0.05f, 0.8f, 1.0f);
        pointLights_[i].attenuation = glm::vec3(1.0f, 0.7f, 1.8f);
        pointLights_[i].modelMtx = glm::scale(glm::translate(glm::mat4(1.0f), position), glm::vec3(calcLightRadius(pointLights_[i].color, pointLights_[i].attenuation)));
    }
    
    spotLights_.resize(1);
    spotLights_[0].color = glm::vec3(1.0f, 1.0f, 1.0f);
    spotLights_[0].phongVals = glm::vec3(0.0f, 1.0f, 1.0f);
    spotLights_[0].attenuation = glm::vec3(1.0f, 0.09f, 0.032f);
    spotLights_[0].cutOff = glm::vec2(glm::cos(glm::radians(12.5f)), glm::cos(glm::radians(17.5f)));
    
    debugVectors_.push_back(glm::mat4(1.0f));    // Set up data for debug vectors.
    glGenVertexArrays(1, &debugVectorsVAO_);
    glBindVertexArray(debugVectorsVAO_);
    glGenBuffers(1, &debugVectorsVBO_);
    glBindBuffer(GL_ARRAY_BUFFER, debugVectorsVBO_);
    glBufferData(GL_ARRAY_BUFFER, debugVectors_.size() * sizeof(glm::mat4), debugVectors_.data(), GL_DYNAMIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, false, sizeof(glm::mat4), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, false, sizeof(glm::mat4), reinterpret_cast<void*>(1 * sizeof(glm::vec4)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, false, sizeof(glm::mat4), reinterpret_cast<void*>(2 * sizeof(glm::vec4)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, false, sizeof(glm::mat4), reinterpret_cast<void*>(3 * sizeof(glm::vec4)));
}

World::~World() {
    glDeleteVertexArrays(1, &debugVectorsVAO_);
    glDeleteBuffers(1, &debugVectorsVBO_);
}

void World::nextTick() {
    sunT_ += sunSpeed_;
    
    lightStates_[0] = (sunlightOn_ ? 1u : 0u);
    lightStates_[1] = (flashlightOn_ ? 1u : 0u);
    for (size_t i = 2; i < pointLights_.size() + 2; ++i) {
        lightStates_[i] = (lampsOn_ ? 1u : 0u);
    }
    for (size_t i = pointLights_.size() + 2; i < Renderer::NUM_LIGHTS; ++i) {
        lightStates_[i] = 0;
    }
    sunPosition_ = glm::vec3(glm::rotate(glm::mat4(1.0f), sunT_, glm::vec3(1.0f, 1.0f, 1.0f)) * glm::vec4(0.0f, 0.0f, Renderer::FAR_PLANE, 1.0f));
    if (sunPosition_.x == 0.0f && sunPosition_.z == 0.0f) {    // Fix edge case when directional light aligns with up vector.
        sunPosition_.x = 0.00001f;
    }
    
    modelTest_.animate(0, glfwGetTime(), modelTestBoneTransforms_);
    characterTest_.update();
    
    debugVectors_.resize(2);
    const ModelRigged::Node* node = characterTest_.model_.findNode("Breast_R");
    if (node != nullptr) {
        debugVectors_[1] = characterTest_.transform_.getTransform() * glm::inverse(characterTest_.model_.globalInverseMtx_) * characterTest_.boneTransforms_[node->boneIndex] * glm::inverse(characterTest_.model_.boneOffsetMatrices_[node->boneIndex]);
    }
    
    /*debugVectors_.resize(2);
    int boneIndex = modelTest_.findNode("head");
    if (boneIndex != -1) {
        debugVectors_[1] = modelTestTransform_.getTransform() * glm::inverse(modelTest_.globalInverseMtx_) * modelTestBoneTransforms_[boneIndex] * glm::inverse(modelTest_.boneOffsetMatrices_[boneIndex]);
    }*/
    
    glBindVertexArray(debugVectorsVAO_);
    glBindBuffer(GL_ARRAY_BUFFER, debugVectorsVBO_);
    int bufferSize;
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);
    if (bufferSize < static_cast<int>(debugVectors_.size() * sizeof(glm::mat4))) {
        glBufferData(GL_ARRAY_BUFFER, debugVectors_.size() * sizeof(glm::mat4), debugVectors_.data(), GL_DYNAMIC_DRAW);
    } else {
        glBufferSubData(GL_ARRAY_BUFFER, 0, debugVectors_.size() * sizeof(glm::mat4), debugVectors_.data());
    }
}
