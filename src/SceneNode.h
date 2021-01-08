#ifndef SCENE_NODE_H_
#define SCENE_NODE_H_

class Scene;
class SceneObject;

#include "Transformable.h"
#include <string>
#include <vector>

using namespace std;

class SceneNode : public Transformable {
    public:
    vector<SceneObject*> objects;
    
    SceneNode* getParentNode() const;
    SceneNode* createChildNode(const string& name = "");
    void attachObject(SceneObject* object);
    
    private:
    string name_;
    Scene* scene_;
    SceneNode* parentNode_;
    vector<SceneNode*> childNodes_;
    
    SceneNode(Scene* scene, SceneNode* parentNode, const string& name = "");
    
    friend class Scene;
};

#endif
