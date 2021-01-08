#ifndef SCENE_OBJECT_
#define SCENE_OBJECT_

class SceneNode;

using namespace std;

class SceneObject {
    public:
    SceneObject();
    SceneNode* getSceneNode() const;
    
    private:
    SceneNode* sceneNode_;
    
    void setSceneNode(SceneNode* sceneNode);
    
    friend class SceneNode;
};

#endif
