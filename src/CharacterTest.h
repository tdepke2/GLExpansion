#ifndef CHARACTER_TEST_H_
#define CHARACTER_TEST_H_

class Shader;

#include "Animation.h"
#include "DrawableInterface.h"
#include "ModelRigged.h"
#include "Transformable.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

class CharacterTest : public DrawableInterface {
    public:
    ModelRigged model_;
    Transformable transform_;
    unordered_map<string, Animation> animations_;
    vector<glm::mat4> boneTransforms_;
    map<int, ModelRigged::DynamicBone> dynamicBones_;
    vector<glm::vec3> activeForces_;
    glm::mat4 lastBoneTransform_;
    
    void init();
    void update();
    void draw(const Shader& shader, const glm::mat4& modelMtx) const;
};

#endif
