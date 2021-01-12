#ifndef SCENE_H_
#define SCENE_H_

class Camera;
class Entity;
class Light;
class SceneNode;
class Shader;

#include "Mesh.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>
#include <memory>
#include <string>
#include <vector>

using namespace std;

class Scene {
    public:
    unique_ptr<Camera> cam_;// hmmm ###########################################
    
    ~Scene();
    SceneNode* getRootNode();
    Camera* createCamera(const string& name = "");
    Light* createLight();
    SceneNode* createSceneNode(const string& name = "");
    Entity* createEntity(const string& name, const string& meshName);
    Entity* createEntity(const string& meshName);
    void generateMesh();
    void generatePlane();
    void generateBezierPatch();
    void generateCube(const string& name, float sideLength = 1.0f);
    void generateSphere(const string& name, float radius = 1.0f, int numSectors = 32, int numStacks = 16);
    void generateCylinder(const string& name, float radiusBase = 1.0f, float radiusTop = 1.0f, float height = 2.0f, int numSectors = 32, int numStacks = 1, bool originAtBase = false);
    
    private:
    string name_;
    unique_ptr<SceneNode> rootNode_;
    vector<unique_ptr<Camera>> cameras_;
    vector<unique_ptr<Light>> lights_;
    vector<unique_ptr<SceneNode>> sceneNodes_;
    vector<unique_ptr<Entity>> entities_;
    map<string, shared_ptr<Mesh>> meshes_;    // may want to move into singleton manager or just make static in Mesh #####################################################################
    
    Scene(const string& name = "");
    void draw(const Shader& shader, const glm::mat4& modelMtx) const;
    
    friend class RenderApp;
};

#endif
