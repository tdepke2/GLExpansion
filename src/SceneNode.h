#ifndef SCENE_NODE_H_
#define SCENE_NODE_H_

class Scene;
class SceneObject;
class Shader;

#include "Transformable.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>

using namespace std;

class SceneNode : public Transformable {
    public:
    Scene* getScene() const;
    SceneNode* getParentNode() const;
    const vector<SceneNode*>& getChildNodes() const;
    const vector<SceneObject*>& getObjects() const;
    SceneNode* createChildNode(const string& name = "");
    void attachObject(SceneObject* object);
    void addChild(SceneNode* child);
    void removeChild(SceneNode* child);
    
    private:
    string name_;
    Scene* scene_;
    SceneNode* parentNode_;
    vector<SceneNode*> childNodes_;
    vector<SceneObject*> objects_;
    
    SceneNode(const string& name, Scene* scene);
    void draw() const;
    
    friend class Scene;
};

#endif
