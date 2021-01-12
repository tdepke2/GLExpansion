#include "Camera.h"
#include "Entity.h"
#include "Light.h"
#include "Scene.h"
#include "SceneNode.h"
#include "Shader.h"
#include <stack>
#include <stdexcept>

Scene::~Scene() {}

SceneNode* Scene::getRootNode() {
    return rootNode_.get();
}

Camera* Scene::createCamera(const string& name) {
    cam_ = unique_ptr<Camera>(new Camera(name));
    return cam_.get();
}

Light* Scene::createLight() {
    return nullptr;
}

SceneNode* Scene::createSceneNode(const string& name) {
    sceneNodes_.emplace_back(new SceneNode(name, this));
    return sceneNodes_.back().get();
}

Entity* Scene::createEntity(const string& name, const string& meshName) {
    auto findResult = meshes_.find(meshName);
    if (findResult == meshes_.end()) {
        throw runtime_error("Unable to find mesh.");
    }
    
    entities_.emplace_back(new Entity(name, findResult->second));
    return entities_.back().get();
}

Entity* Scene::createEntity(const string& meshName) {
    return createEntity("", meshName);
}

void Scene::generateMesh() {
    
}

void Scene::generatePlane() {
    
}

void Scene::generateBezierPatch() {
    
}

void Scene::generateCube(const string& name, float sideLength) {
    if (meshes_.count(name) != 0) {
        throw runtime_error("Name already used.");
    }
    
    shared_ptr<Mesh> cube = make_shared<Mesh>();
    cube->generateCube(sideLength);
    meshes_.emplace(name, cube);
}

void Scene::generateSphere(const string& name, float radius, int numSectors, int numStacks) {
    if (meshes_.count(name) != 0) {
        throw runtime_error("Name already used.");
    }
    
    shared_ptr<Mesh> sphere = make_shared<Mesh>();
    sphere->generateSphere(radius, numSectors, numStacks);
    meshes_.emplace(name, sphere);
}

void Scene::generateCylinder(const string& name, float radiusBase, float radiusTop, float height, int numSectors, int numStacks, bool originAtBase) {
    if (meshes_.count(name) != 0) {
        throw runtime_error("Name already used.");
    }
    
    shared_ptr<Mesh> cylinder = make_shared<Mesh>();
    cylinder->generateCylinder(radiusBase, radiusTop, height, numSectors, numStacks, originAtBase);
    meshes_.emplace(name, cylinder);
}

Scene::Scene(const string& name) :
    name_(name),
    rootNode_(unique_ptr<SceneNode>(new SceneNode("root", this))) {
}

void Scene::draw(const Shader& shader, const glm::mat4& modelMtx) const {
    stack<SceneNode*> nodeStack;
    nodeStack.push(rootNode_.get());
    stack<glm::mat4> modelMtxStack;
    modelMtxStack.emplace(1.0f);
    
    while (!nodeStack.empty()) {
        SceneNode* topNode = nodeStack.top();
        nodeStack.pop();
        glm::mat4 topModelMtx = modelMtxStack.top();
        modelMtxStack.pop();
        
        topModelMtx *= topNode->getTransform();
        shader.setMat4("modelMtx", topModelMtx);
        topNode->draw();
        
        for (SceneNode* node : topNode->getChildNodes()) {
            nodeStack.push(node);
            modelMtxStack.push(topModelMtx);
        }
    }
}
