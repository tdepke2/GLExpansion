#ifndef WORLD_H_
#define WORLD_H_

#include "CharacterTest.h"
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
    glm::vec3 color;
    glm::vec3 phongVals;
};

struct PointLight {
    glm::mat4 modelMtx;
    glm::vec3 color;
    glm::vec3 phongVals;
    glm::vec3 attenuation;
};

struct SpotLight {
    glm::mat4 modelMtx;
    glm::vec3 color;
    glm::vec3 phongVals;
    glm::vec3 attenuation;
    glm::vec2 cutOff;
};

class World {
    public:
    static constexpr unsigned int ATTRIBUTE_LOCATION_V_MODEL_MTX = 5;
    static constexpr unsigned int ATTRIBUTE_LOCATION_V_COLOR = 9;
    static constexpr unsigned int ATTRIBUTE_LOCATION_V_PHONG_VALS = 10;
    static constexpr unsigned int ATTRIBUTE_LOCATION_V_ATTENUATION = 11;
    
    Mesh lightCube_, lightSphere_, lightCone_, cube1_;
    ModelStatic sceneTest_;
    ModelRigged modelTest_;
    vector<glm::mat4> modelTestBoneTransforms_;
    CharacterTest characterTest_;
    Transformable sceneTestTransform_, modelTestTransform_;
    DirectionalLight sunLight_;
    vector<PointLight> pointLights_;
    vector<SpotLight> spotLights_;
    unsigned int lightStates_[Renderer::NUM_LIGHTS];
    bool flashlightOn_, sunlightOn_, lampsOn_;
    float sunT_, sunSpeed_;
    glm::vec3 sunPosition_;
    unsigned int debugVectorsVAO_, debugVectorsVBO_;
    vector<glm::mat4> debugVectors_;
    
    static float calcLightRadius(const glm::vec3& color, const glm::vec3& attenuation);    // Determine the maximum bounds of a light source given the color and attenuation factors.
    World();
    ~World();
    void nextTick();
    
    private:
};

#endif
