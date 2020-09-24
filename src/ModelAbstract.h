#ifndef MODEL_ABSTRACT_H_
#define MODEL_ABSTRACT_H_

class Animation;
class Shader;

#include "DrawableInterface.h"
#include "Mesh.h"
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

class ModelAbstract : public DrawableInterface {
    public:
    static constexpr bool VERBOSE_OUTPUT_ = true;
    vector<Mesh> meshes_;
    vector<glm::mat4> meshTransforms_;
    string directoryPath_;
    
    ModelAbstract();
    virtual ~ModelAbstract();
    virtual void loadFile(const string& filename, unordered_map<string, Animation>* animations = nullptr) = 0;
    virtual void applyInstanceBuffer(unsigned int startIndex) const;
    virtual void draw(const Shader& shader, const glm::mat4& modelMtx) const;
    virtual void drawGeometry() const;
    virtual void drawGeometry(const Shader& shader, const glm::mat4& modelMtx) const;
    virtual void drawInstanced(unsigned int count) const;
    virtual void drawGeometryInstanced(unsigned int count) const;
    
    protected:
    static inline glm::vec3 castVec3(const aiVector3D& v) { return glm::vec3(v.x, v.y, v.z); }
    static inline glm::vec2 castVec2(const aiVector2D& v) { return glm::vec2(v.x, v.y); }
    static inline glm::quat castQuat(const aiQuaternion& q) { return glm::quat(q.w, q.x, q.y, q.z); }
    static inline glm::mat4 castMat4(const aiMatrix4x4& m) { return glm::transpose(glm::make_mat4(&m.a1)); }
    static inline glm::mat3 castMat3(const aiMatrix3x3& m) { return glm::transpose(glm::make_mat3(&m.a1)); }
    const aiScene* loadScene(Assimp::Importer* importer, const string& filename);    // Attempts to load the model file into an assimp scene. Returns nullptr on fail.
    void loadMaterialTextures(aiMaterial* material, aiTextureType type, const string& uniformName, unsigned int index, vector<Mesh::Texture>& textures);    // Adds material textures to the textures vector if they exist.
    template<typename V>
    void processMeshAttributes(aiMesh* mesh, const aiScene* scene, vector<V>& vertices, vector<unsigned int>& indices, vector<Mesh::Texture>& textures);    // Loads vertices, indices, and textures for a single mesh.
};

#endif
