#ifndef _MODEL_RIGGED_H
#define _MODEL_RIGGED_H

#include "Animation.h"
#include "MeshRigged.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

class ModelRigged {
    public:
    static constexpr bool VERBOSE_OUTPUT = true;
    
    struct Node {
        Node* parent;
        vector<Node*> children;
        string name;
        unsigned int id;
        int boneIndex;
        glm::mat4 transform;
        
        Node(Node* parent, const string& name, unsigned int id, const glm::mat4& transform) : parent(parent), name(name), id(id), boneIndex(-1), transform(transform) {}
    };
    
    vector<MeshRigged> meshes;
    vector<Animation> animations;
    Node* rootNode;
    unsigned int numNodes;
    vector<glm::mat4> boneOffsetMatrices;
    glm::mat4 globalInverseMtx;
    string directoryPath;
    
    ModelRigged();
    ModelRigged(const string& filename, const glm::mat4& transformMtx = glm::mat4(1.0f));
    ~ModelRigged();
    void loadFile(const string& filename, const glm::mat4& transformMtx = glm::mat4(1.0f));
    void animate(const Shader* shader, unsigned int animationIndex, double time, vector<glm::mat4>& boneTransforms);
    void draw() const;
    void draw(const Shader& shader) const;
    
    private:
    static inline glm::vec3 _castVec3(const aiVector3D& v) { return glm::vec3(v.x, v.y, v.z); }
    static inline glm::vec2 _castVec2(const aiVector2D& v) { return glm::vec2(v.x, v.y); }
    static inline glm::quat _castQuat(const aiQuaternion& q) { return glm::quat(q.w, q.x, q.y, q.z); }
    static inline glm::mat4 _castMat4(const aiMatrix4x4& m) { return glm::transpose(glm::make_mat4(&m.a1)); }
    static inline glm::mat3 _castMat3(const aiMatrix3x3& m) { return glm::transpose(glm::make_mat3(&m.a1)); }
    Node* _processNode(Node* parent, aiNode* node, const aiScene* scene, unordered_map<string, unsigned int>& boneMapping, const glm::mat4& transformMtx);
    MeshRigged _processMesh(aiMesh* mesh, const aiScene* scene, unordered_map<string, unsigned int>& boneMapping, const glm::mat4& transformMtx);
    void _loadMaterialTextures(aiMaterial* material, aiTextureType type, const string& uniformName, unsigned int index, vector<MeshRigged::Texture>& textures);
    void _animateNodes(const Node* node, const Animation* animation, double animationTime, glm::mat4 parentTransform, vector<glm::mat4>& boneTransforms) const;
};

#endif
