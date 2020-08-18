#ifndef WORLD_H_
#define WORLD_H_

#include "ModelRigged.h"
#include "ModelStatic.h"
#include "Transformable.h"

using namespace std;

class World {
    public:
    Mesh lightCube, cube1, sphere1;
    ModelStatic sceneTest;
    ModelRigged modelTest;
    Transformable sceneTestTransform, modelTestTransform;
    
    World();
    ~World();
    
    private:
    
};

#endif
