#include "Scene.h"
#include "SceneNode.h"
#include "SceneObject.h"
#include "Shader.h"
#include <stdexcept>

Scene* SceneNode::getScene() const {
    return scene_;
}

SceneNode* SceneNode::getParentNode() const {
    return parentNode_;
}

const vector<SceneNode*>& SceneNode::getChildNodes() const {
    return childNodes_;
}
const vector<SceneObject*>& SceneNode::getObjects() const {
    return objects_;
}

SceneNode* SceneNode::createChildNode(const string& name) {
    SceneNode* child = scene_->createSceneNode(name);
    addChild(child);
    return child;
}

void SceneNode::attachObject(SceneObject* object) {
    objects_.push_back(object);
    object->setSceneNode(this);
}

void SceneNode::addChild(SceneNode* child) {
    if (child->parentNode_ == nullptr) {
        child->parentNode_ = this;
        childNodes_.push_back(child);
    } else {
        throw runtime_error("Child already has parent.");
    }
}

void SceneNode::removeChild(SceneNode* child) {
    if (child->parentNode_ == this) {
        for (size_t i = 0; i < childNodes_.size(); ++i) {
            if (childNodes_[i] == child) {
                child->parentNode_ = nullptr;
                childNodes_[i] = childNodes_.back();
                childNodes_.pop_back();
                return;
            }
        }
    } else {
        throw runtime_error("Node does not have this child.");
    }
}

SceneNode::SceneNode(const string& name, Scene* scene) :
    name_(name),
    scene_(scene),
    parentNode_(nullptr) {
}

void SceneNode::draw() const {
    for (const SceneObject* object : objects_) {
        object->draw();
    }
}
