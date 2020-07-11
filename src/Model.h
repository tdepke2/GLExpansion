#ifndef _MODEL_H
#define _MODEL_H

#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <string>
#include <vector>

using namespace std;

class Model {
    public:
    static constexpr bool VERBOSE_OUTPUT = true;
    vector<Mesh> meshes;
    string directoryPath;
    
    Model();
    Model(const string& filename, const glm::mat4& transformMtx = glm::mat4(1.0f));
    void loadFile(const string& filename, const glm::mat4& transformMtx = glm::mat4(1.0f));
    void applyInstanceBuffer(unsigned int startIndex) const;
    void draw() const;
    void draw(const Shader& shader) const;
    void drawInstanced(unsigned int count) const;
    void drawInstanced(const Shader& shader, unsigned int count) const;
    
    protected:
    virtual void _processNode(aiNode* node, const aiScene* scene, const glm::mat4& transformMtx);
    virtual Mesh _processMesh(aiMesh* mesh, const aiScene* scene, const glm::mat4& transformMtx);
    virtual void _loadMaterialTextures(aiMaterial* material, aiTextureType type, const string& uniformName, unsigned int index, vector<Mesh::Texture>& textures);
};

#endif
