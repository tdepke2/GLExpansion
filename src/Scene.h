#ifndef SCENE_H_
#define SCENE_H_

class Camera;
class Entity;
class Light;
class SceneNode;

#include <memory>
#include <string>
#include <vector>

using namespace std;

class Scene {
    public:
    unique_ptr<Camera> cam_;// hmmm ###########################################
    
    ~Scene();
    SceneNode* getRootNode();
    Camera* createCamera();
    Light* createLight();
    SceneNode* createSceneNode(SceneNode* parentNode, const string& name = "");
    Entity* createEntity();
    
    private:
    string name_;
    unique_ptr<SceneNode> rootNode_;
    vector<unique_ptr<SceneNode>> sceneNodes_;
    
    Scene(const string& name = "");
    
    friend class Renderer;
};

#endif
