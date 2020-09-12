#ifndef CHARACTER_TEST_H_
#define CHARACTER_TEST_H_

class Shader;

#include "DrawableInterface.h"
#include "ModelRigged.h"
#include "Transformable.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>
#include <vector>

using namespace std;

class CharacterTest : public DrawableInterface {
    public:
    ModelRigged model_;
    Transformable transform_;
    vector<glm::mat4> boneTransforms_;
    vector<glm::vec3> activeForces_;
    glm::mat4 lastBoneTransform_;
    
    void init();
    void update();
    glm::quat findRotationBetweenVectors(glm::vec3 source, glm::vec3 destination) const;
    void draw(const Shader& shader, const glm::mat4& modelMtx) const;
};

#endif
