#include "Camera.h"
#include "Entity.h"
#include "Light.h"
#include "Scene.h"
#include "SceneNode.h"

Scene::~Scene() {}

SceneNode* Scene::getRootNode() {
    return rootNode_.get();
}

Camera* Scene::createCamera() {
    cam_ = unique_ptr<Camera>(new Camera);
    return cam_.get();
}

Light* Scene::createLight() {
    return nullptr;
}

SceneNode* Scene::createSceneNode(SceneNode* parentNode, const string& name) {
    sceneNodes_.emplace_back(new SceneNode(this, parentNode, name));
    return sceneNodes_.back().get();
}

Entity* Scene::createEntity() {
    return nullptr;
}

Scene::Scene(const string& name) :
    name_(name),
    rootNode_(unique_ptr<SceneNode>(new SceneNode(this, nullptr, "root"))) {
}
