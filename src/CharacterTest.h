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
    ModelRigged model_, model2_;
    Transformable transform_, transform2_, transform3_, transform4_;
    unordered_map<string, Animation> animations_, animations2_;
    vector<glm::mat4> boneTransforms_, boneTransforms2_, boneTransforms3_, boneTransforms4_;
    map<int, ModelRigged::DynamicBone> dynamicBones_;
    vector<glm::vec3> activeForces_;
    
    void init();
    void update();
    void draw(const Shader& shader, const glm::mat4& modelMtx) const;
};

#endif
