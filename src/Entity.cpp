#include "Entity.h"
#include "Mesh.h"

const shared_ptr<Mesh>& Entity::getMesh() const {
    return mesh_;
}

void Entity::draw() const {
    mesh_->drawGeometry();
}

Entity::Entity(const string& name, shared_ptr<Mesh> mesh) :
    SceneObject(name),
    mesh_(mesh) {
}
