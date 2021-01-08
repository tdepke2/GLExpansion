#include "SceneNode.h"
#include "SceneObject.h"

SceneObject::SceneObject() :
    sceneNode_(nullptr) {
}

SceneNode* SceneObject::getSceneNode() const {
    return sceneNode_;
}

void SceneObject::setSceneNode(SceneNode* sceneNode) {
    sceneNode_ = sceneNode;
}
