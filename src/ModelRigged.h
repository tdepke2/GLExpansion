#ifndef MODEL_RIGGED_H_
#define MODEL_RIGGED_H_

#include "Animation.h"
#include "ModelAbstract.h"
#include <unordered_map>

using namespace std;

class ModelRigged : public ModelAbstract {
    public:
    struct Node {
        Node* parent;
        vector<Node*> children;
        string name;
        unsigned int id;
        int boneIndex;
        glm::mat4 transform;
        
        Node(Node* parent, const string& name, unsigned int id, const glm::mat4& transform) : parent(parent), name(name), id(id), boneIndex(-1), transform(transform) {}
    };
    
    vector<Animation> animations_;
    Node* rootNode_;
    unsigned int numNodes_;
    vector<glm::mat4> boneOffsetMatrices_;
    glm::mat4 globalInverseMtx_;
    
    ModelRigged();
    ModelRigged(const string& filename);
    ~ModelRigged();
    void loadFile(const string& filename);
    void animate(const Shader& shader, unsigned int animationIndex, double time, vector<glm::mat4>& boneTransforms);
    
    private:
    Node* processNode(Node* parent, aiNode* node, const aiScene* scene, unordered_map<string, unsigned int>& boneMapping);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene, unordered_map<string, unsigned int>& boneMapping);
    void animateNodes(const Node* node, const Animation& animation, double animationTime, const glm::mat4& parentTransform, vector<glm::mat4>& boneTransforms) const;
};

#endif
