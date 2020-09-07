#ifndef CHARACTER_TEST_H_
#define CHARACTER_TEST_H_

class Shader;

#include "DrawableInterface.h"
#include "ModelRigged.h"
#include "Transformable.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

class CharacterTest : public DrawableInterface {
    public:
    ModelRigged model_;
    Transformable transform_;
    
    void init();
    void draw(const Shader& shader, const glm::mat4& modelMtx) const;
};

#endif
