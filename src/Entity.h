#ifndef ENTITY_H_
#define ENTITY_H_

class Mesh;

#include "SceneObject.h"
#include <memory>
#include <string>

using namespace std;

class Entity : public SceneObject {
    public:
    const shared_ptr<Mesh>& getMesh() const;
    
    protected:
    void draw() const;
    
    private:
    shared_ptr<Mesh> mesh_;
    
    Entity(const string& name, shared_ptr<Mesh> mesh);
    
    friend class Scene;
};

#endif
