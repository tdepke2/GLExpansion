#ifndef WORLD_H_
#define WORLD_H_

#include "ModelRigged.h"
#include "ModelStatic.h"
#include "Renderer.h"
#include "Transformable.h"

using namespace std;

struct DirectionalLight {
    glm::vec3 direction;
    glm::vec3 ambient, diffuse, specular;
};

struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient, diffuse, specular;
    glm::vec3 attenuationVals;
};

struct SpotLight {
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 ambient, diffuse, specular;
    glm::vec3 attenuationVals;
    glm::vec2 cutOff;
};

class World {
    public:
    Mesh lightCube_, cube1_, sphere1_;
    ModelStatic sceneTest_;
    ModelRigged modelTest_;
    Transformable sceneTestTransform_, modelTestTransform_;
    DirectionalLight sunLight_;
    vector<PointLight> pointLights_;
    vector<SpotLight> spotLights_;
    unsigned int lightStates_[Renderer::NUM_LIGHTS];
    bool flashlightOn_, sunlightOn_, lampsOn_;
    float sunT_, sunSpeed_;
    glm::vec3 sunPosition_;
    
    World();
    ~World();
    void nextTick();
    
    private:
    
};

#endif
