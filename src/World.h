#ifndef WORLD_H_
#define WORLD_H_

#include "ModelRigged.h"
#include "ModelStatic.h"
#include "Renderer.h"
#include "Transformable.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

using namespace std;

struct DirectionalLight {
    glm::vec3 direction;
    glm::vec3 ambient, diffuse, specular;
};

struct PointLight {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec3 phongVals;
    glm::vec3 attenuation;
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
    static constexpr unsigned int ATTRIBUTE_LOCATION_V_TRANSLATION = 5;
    static constexpr unsigned int ATTRIBUTE_LOCATION_V_COLOR = 6;
    static constexpr unsigned int ATTRIBUTE_LOCATION_V_PHONG_VALS = 7;
    static constexpr unsigned int ATTRIBUTE_LOCATION_V_ATTENUATION = 8;
    
    Mesh lightCube_, lightSphere_, cube1_, sphere1_;
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
