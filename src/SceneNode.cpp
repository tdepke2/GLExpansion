#include "Scene.h"
#include "SceneNode.h"
#include "SceneObject.h"

SceneNode* SceneNode::createChildNode(const string& name) {
    SceneNode* child = scene_->createSceneNode(this, name);
    childNodes_.push_back(child);
    return child;
}

void SceneNode::attachObject(SceneObject* object) {
    objects.push_back(object);
    object->setSceneNode(this);
}

SceneNode::SceneNode(Scene* scene, SceneNode* parentNode, const string& name) :
    scene_(scene),
    parentNode_(parentNode),
    name_(name) {
}
