#ifndef MODEL_RIGGED_H_
#define MODEL_RIGGED_H_

#include "Animation.h"
#include "DampedSpringMotion.h"
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
    struct DynamicBone {
        glm::vec3 lastPosition;
        glm::vec3 lastRotation;
        glm::vec3 linearVel;
        glm::vec3 angularVel;
        glm::vec3 linearAcc;
        glm::quat angularAcc;
        DampedSpringMotion springMotion;
        
        DynamicBone() : lastPosition(0.0f), lastRotation(0.0f), linearVel(0.0f), angularVel(0.0f), linearAcc(0.0f), angularAcc(1.0f, 0.0f, 0.0f, 0.0f) {}
    };
    
    static constexpr unsigned int MAX_NUM_BONES = 128;
    
    vector<Animation> animations_;
    vector<glm::mat4> boneOffsetMatrices_;
    
    ModelRigged();
    ModelRigged(const string& filename);
    ~ModelRigged();
    Node* getRootNode() const;
    unsigned int getNumNodes() const;
    const glm::mat4& getGlobalInverseMtx() const;
    void loadFile(const string& filename);
    void ragdoll(const glm::mat4& modelMtx, map<int, DynamicBone>& dynamicBones, vector<glm::mat4>& boneTransforms) const;
    void animate(unsigned int animationIndex, double time, vector<glm::mat4>& boneTransforms) const;    // Sets the transforms in the boneTransforms vector to match the given animation at the specified time.
    const Node* findNode(const string& nodeName) const;
    
    private:
    Node* rootNode_;
    unsigned int numNodes_;
    glm::mat4 globalInverseMtx_;
    
    Node* processNode(Node* parent, aiNode* node, glm::mat4 combinedTransform, const aiScene* scene, unordered_map<string, uint8_t>& boneMapping);    // Recursively traverse the scene nodes while adding mesh data. This also builds the node tree and maps bone names.
    Mesh processMesh(aiMesh* mesh, const aiScene* scene, unordered_map<string, uint8_t>& boneMapping);    // Generate a new mesh and collect any new bone data.
    void ragdollNodes(const Node* node, const glm::mat4& modelMtx, map<int, DynamicBone>& dynamicBones, glm::mat4 combinedTransform, vector<glm::mat4>& boneTransforms) const;
    void animateNodes(const Node* node, const Animation& animation, double animationTime, glm::mat4 combinedTransform, vector<glm::mat4>& boneTransforms) const;    // Recursively traverse the nodes to set each bone transform.
    glm::quat findRotationBetweenVectors(glm::vec3 source, glm::vec3 destination) const;
};

#endif
