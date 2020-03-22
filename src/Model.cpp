#include "Model.h"
#include "Simulator.h"
#include <cassert>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <utility>

Model::Model() {}

Model::Model(const string& filename) {
    loadFile(filename);
}

void Model::loadFile(const string& filename) {
    assert(meshes.empty());
    if (VERBOSE_OUTPUT) {
        cout << "Loading model \"" << filename << "\".\n";
    }
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_FlipUVs);
    
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        cout << "Failed to load model file \"" << filename << "\": " << importer.GetErrorString() << "\n";
        return;
    }
    directoryPath = filename.substr(0, filename.find_last_of('/'));
    
    if (VERBOSE_OUTPUT) {
        cout << "  Number of animations: " << scene->mNumAnimations << "\n";
        cout << "  Number of cameras:    " << scene->mNumCameras << "\n";
        cout << "  Number of lights:     " << scene->mNumLights << "\n";
        cout << "  Number of materials:  " << scene->mNumMaterials << "\n";
        cout << "  Number of meshes:     " << scene->mNumMeshes << "\n";
        cout << "  Number of textures:   " << scene->mNumTextures << "\n";
    }
    meshes.reserve(scene->mNumMeshes);
    _processNode(scene->mRootNode, scene);
}

void Model::draw() const {
    for (size_t i = 0; i < meshes.size(); ++i) {
        meshes[i].draw();
    }
}

void Model::draw(const Shader& shader) const {
    for (size_t i = 0; i < meshes.size(); ++i) {
        meshes[i].draw(shader);
    }
}

void Model::_processNode(aiNode* node, const aiScene* scene) {
    if (VERBOSE_OUTPUT) {
        cout << "  Node " << node->mName.C_Str() << " has " << node->mNumMeshes << " meshes and " << node->mNumChildren << " children.\n";
    }
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {    // Process all meshes in this node.
        meshes.push_back(_processMesh(scene->mMeshes[node->mMeshes[i]], scene));
    }
    
    for (unsigned int i = 0; i < node->mNumChildren; ++i) {    // Process all child nodes.
        _processNode(node->mChildren[i], scene);
    }
}

Mesh Model::_processMesh(aiMesh* mesh, const aiScene* scene) {
    if (VERBOSE_OUTPUT) {
        cout << "    Mesh " << mesh->mName.C_Str() << " has " << mesh->mNumBones << " bones, " << mesh->mNumFaces << " faces, and " << mesh->mNumVertices << " vertices.\n";
    }
    vector<Mesh::Vertex> vertices;
    vertices.reserve(mesh->mNumVertices);
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        glm::vec3 vPosition(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        glm::vec3 vNormal(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        glm::vec2 vTexCoords;
        if (mesh->mTextureCoords[0]) {
            vTexCoords.x = mesh->mTextureCoords[0][i].x;
            vTexCoords.y = mesh->mTextureCoords[0][i].y;
        } else {
            vTexCoords.x = 0.0f;
            vTexCoords.y = 0.0f;
        }
        
        vertices.emplace_back(vPosition.x, vPosition.y, vPosition.z, vNormal.x, vNormal.y, vNormal.z, vTexCoords.x, vTexCoords.y);
    }
    
    vector<unsigned int> indices;
    indices.reserve(mesh->mNumFaces * 3);
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; ++j) {
            indices.push_back(mesh->mFaces[i].mIndices[j]);
        }
    }
    
    vector<Mesh::Texture> textures;
    _loadMaterialTextures(scene->mMaterials[mesh->mMaterialIndex], aiTextureType_DIFFUSE, "material.texDiffuse", textures);
    _loadMaterialTextures(scene->mMaterials[mesh->mMaterialIndex], aiTextureType_SPECULAR, "material.texSpecular", textures);
    
    return Mesh(move(vertices), move(indices), move(textures));
}

void Model::_loadMaterialTextures(aiMaterial* material, aiTextureType type, const string& uniformName, vector<Mesh::Texture>& textures) {
    if (VERBOSE_OUTPUT) {
        cout << "      Material " << uniformName << " has " << material->GetTextureCount(type) << " textures.\n";
    }
    for (unsigned int i = 0; i < material->GetTextureCount(type); ++i) {
        aiString str;
        material->GetTexture(type, i, &str);
        textures.emplace_back(Simulator::loadTexture(directoryPath + "/" + string(str.C_Str())), uniformName + to_string(i));
    }
}
