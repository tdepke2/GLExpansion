#ifndef MODEL_RIGGED_H_
#define MODEL_RIGGED_H_

#include "Animation.h"
#include "ModelAbstract.h"
#include <map>
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
    
    static constexpr unsigned int MAX_NUM_BONES = 128;
    
    vector<Animation> animations_;
    Node* rootNode_;
    unsigned int numNodes_;
    vector<glm::mat4> boneOffsetMatrices_;
    glm::mat4 globalInverseMtx_;
    
    ModelRigged();
    ModelRigged(const string& filename);
    ~ModelRigged();
    void loadFile(const string& filename);
    void animate(unsigned int animationIndex, double time, vector<glm::mat4>& boneTransforms) const;    // Sets the transforms in the boneTransforms vector to match the given animation at the specified time.
    void animate2(map<int, unsigned int>& physicsBones, double time, vector<glm::mat4>& boneTransforms) const;
    int findBoneIndex(const string& boneName) const;
    
    private:
    Node* processNode(Node* parent, aiNode* node, glm::mat4 combinedTransform, const aiScene* scene, unordered_map<string, uint8_t>& boneMapping);    // Recursively traverse the scene nodes while adding mesh data. This also builds the node tree and maps bone names.
    Mesh processMesh(aiMesh* mesh, const aiScene* scene, unordered_map<string, uint8_t>& boneMapping);    // Generate a new mesh and collect any new bone data.
    void animateNodes(const Node* node, const Animation& animation, double animationTime, glm::mat4 combinedTransform, vector<glm::mat4>& boneTransforms) const;    // Recursively traverse the nodes to set each bone transform.
    void animateNodes2(const Node* node, map<int, unsigned int>& physicsBones, double time, glm::mat4 combinedTransform, vector<glm::mat4>& boneTransforms) const;
};

#endif
