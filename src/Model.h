#ifndef _MODEL_H
#define _MODEL_H

#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <vector>

using namespace std;

class Model {
    public:
    static constexpr bool VERBOSE_OUTPUT = true;
    vector<Mesh> meshes;
    string directoryPath;
    
    Model();
    Model(const string& filename);
    void loadFile(const string& filename);
    void draw() const;
    void draw(const Shader& shader) const;
    
    private:
    void _processNode(aiNode* node, const aiScene* scene);
    Mesh _processMesh(aiMesh* mesh, const aiScene* scene);
    void _loadMaterialTextures(aiMaterial* material, aiTextureType type, const string& uniformName, vector<Mesh::Texture>& textures);
};

#endif
