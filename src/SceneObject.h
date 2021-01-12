#ifndef SCENE_OBJECT_
#define SCENE_OBJECT_

class SceneNode;

#include <string>

using namespace std;

class SceneObject {
    public:
    SceneObject(const string& name);
    virtual ~SceneObject();
    const string& getName() const;
    SceneNode* getSceneNode() const;
    
    protected:
    virtual void draw() const = 0;
    
    private:
    string name_;
    SceneNode* sceneNode_;
    
    void setSceneNode(SceneNode* sceneNode);
    
    friend class SceneNode;
};

#endif
