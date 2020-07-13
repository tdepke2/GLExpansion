#include "ModelStatic.h"
#include "Simulator.h"
#include <cassert>
#include <iostream>
#include <utility>

ModelStatic::ModelStatic() {}

ModelStatic::ModelStatic(const string& filename) {
    loadFile(filename);
}

void ModelStatic::loadFile(const string& filename) {
    Assimp::Importer importer;
    const aiScene* scene = loadScene(&importer, filename);
    if (scene == nullptr) {
        return;
    }
    modelMtx_ = castMat4(scene->mRootNode->mTransformation);
    meshes_.reserve(scene->mNumMeshes);
    processNode(scene->mRootNode, scene);
}

void ModelStatic::processNode(aiNode* node, const aiScene* scene) {
    glm::mat4 thisTransformMtx = castMat4(node->mTransformation);
    if (VERBOSE_OUTPUT) {
        cout << "  Node " << node->mName.C_Str() << " has " << node->mNumMeshes << " meshes and " << node->mNumChildren << " children.\n";
        if (thisTransformMtx == glm::mat4(1.0f)) {
            cout << "  Transform: IdentityMtx\n";
        } else {
            cout << "  Transform: " << glm::to_string(thisTransformMtx) << "\n";
        }
    }
    
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {    // Process all meshes in this node.
        meshes_.push_back(processMesh(scene->mMeshes[node->mMeshes[i]], scene));
    }
    for (unsigned int i = 0; i < node->mNumChildren; ++i) {    // Process all child nodes.
        processNode(node->mChildren[i], scene);
    }
}

Mesh ModelStatic::processMesh(aiMesh* mesh, const aiScene* scene) {
    vector<Mesh::Vertex> vertices;
    vector<unsigned int> indices;
    vector<Mesh::Texture> textures;
    processMeshAttributes(mesh, scene, vertices, indices, textures);
    
    return Mesh(move(vertices), move(indices), move(textures));
}
