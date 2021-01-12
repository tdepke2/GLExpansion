#include "SceneNode.h"
#include "SceneObject.h"

SceneObject::SceneObject(const string& name) :
    name_(name),
    sceneNode_(nullptr) {
}

SceneObject::~SceneObject() {}

const string& SceneObject::getName() const {
    return name_;
}

SceneNode* SceneObject::getSceneNode() const {
    return sceneNode_;
}

void SceneObject::setSceneNode(SceneNode* sceneNode) {
    sceneNode_ = sceneNode;
}
