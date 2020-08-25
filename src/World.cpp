#include "World.h"

World::World() :
    flashlightOn_(false),
    sunlightOn_(true),
    lampsOn_(false),
    sunT_(0.0f),
    sunSpeed_(0.016f) {
    
    lightCube_.generateCube(0.2f);
    cube1_.generateCube();
    sphere1_.generateSphere();
    
    sceneTest_.loadFile("models/boot_camp/boot_camp.obj");
    sceneTestTransform_.setScale(glm::vec3(0.025f, 0.025f, 0.025f));
    sceneTestTransform_.setPitchYawRoll(glm::vec3(-glm::pi<float>() / 2.0f, 0.0f, 0.0f));
    
    modelTest_.loadFile("models/bob_lamp_update/bob_lamp_update.md5mesh");
    //modelTest_.loadFile("models/hellknight/hellknight.md5mesh");
    //modelTest_.loadFile("models/spaceship/Intergalactic Spaceship_Blender_2.8_Packed textures.dae");
    modelTestTransform_.setScale(glm::vec3(0.3f));
    modelTestTransform_.setPosition(glm::vec3(0.0f, 0.0f, 2.0f));
    
    sunLight_.ambient = glm::vec3(1.0f, 1.0f, 1.0f) * 0.05f;
    sunLight_.diffuse = glm::vec3(1.0f, 1.0f, 1.0f) * 0.4f;
    sunLight_.specular = glm::vec3(1.0f, 1.0f, 1.0f) * 0.5f;
    
    pointLights_.resize(4);
    pointLights_[0].position = glm::vec3(0.7f, 0.2f, 2.0f);
    pointLights_[0].ambient = glm::vec3(5.0f, 5.0f, 5.0f) * 0.05f;
    pointLights_[0].diffuse = glm::vec3(5.0f, 5.0f, 5.0f) * 0.8f;
    pointLights_[0].specular = glm::vec3(5.0f, 5.0f, 5.0f);
    pointLights_[0].attenuationVals = glm::vec3(1.0f, 0.09f, 0.032f);
    
    pointLights_[1].position = glm::vec3(2.3f, -3.3f, -4.0f);
    pointLights_[1].ambient = glm::vec3(10.0f, 0.0f, 0.0f) * 0.05f;
    pointLights_[1].diffuse = glm::vec3(10.0f, 0.0f, 0.0f) * 0.8f;
    pointLights_[1].specular = glm::vec3(10.0f, 0.0f, 0.0f);
    pointLights_[1].attenuationVals = glm::vec3(1.0f, 0.09f, 0.032f);
    
    pointLights_[2].position = glm::vec3(-4.0f, 2.0f, -12.0f);
    pointLights_[2].ambient = glm::vec3(0.0f, 0.0f, 15.0f) * 0.05f;
    pointLights_[2].diffuse = glm::vec3(0.0f, 0.0f, 15.0f) * 0.8f;
    pointLights_[2].specular = glm::vec3(0.0f, 0.0f, 15.0f);
    pointLights_[2].attenuationVals = glm::vec3(1.0f, 0.09f, 0.032f);
    
    pointLights_[3].position = glm::vec3(0.0f, 0.0f, -3.0f);
    pointLights_[3].ambient = glm::vec3(0.0f, 5.0f, 0.0f) * 0.05f;
    pointLights_[3].diffuse = glm::vec3(0.0f, 5.0f, 0.0f) * 0.8f;
    pointLights_[3].specular = glm::vec3(0.0f, 5.0f, 0.0f);
    pointLights_[3].attenuationVals = glm::vec3(1.0f, 0.09f, 0.032f);
    
    spotLights_.resize(1);
    spotLights_[0].ambient = glm::vec3(1.0f, 1.0f, 1.0f) * 0.0f;
    spotLights_[0].diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
    spotLights_[0].specular = glm::vec3(1.0f, 1.0f, 1.0f);
    spotLights_[0].attenuationVals = glm::vec3(1.0f, 0.09f, 0.032f);
    spotLights_[0].cutOff = glm::vec2(glm::cos(glm::radians(12.5f)), glm::cos(glm::radians(17.5f)));
}

World::~World() {
    
}

void World::nextTick() {
    sunT_ += sunSpeed_;
    
    lightStates_[0] = (sunlightOn_ ? 1u : 0u);
    lightStates_[1] = (flashlightOn_ ? 1u : 0u);
    for (int i = 0; i < 4; ++i) {
        lightStates_[i + 2] = (lampsOn_ ? 1u : 0u);
    }
    for (int i = 6; i < Renderer::NUM_LIGHTS; ++i) {
        lightStates_[i] = 0;
    }
    sunPosition_ = glm::vec3(glm::rotate(glm::mat4(1.0f), sunT_, glm::vec3(1.0f, 1.0f, 1.0f)) * glm::vec4(0.0f, 0.0f, Renderer::FAR_PLANE, 1.0f));
    if (sunPosition_.x == 0.0f && sunPosition_.z == 0.0f) {    // Fix edge case when directional light aligns with up vector.
        sunPosition_.x = 0.00001f;
    }
}
