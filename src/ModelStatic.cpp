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
    meshes_.reserve(scene->mNumMeshes);
    meshTransforms_.reserve(scene->mNumMeshes);
    processNode(scene->mRootNode, glm::mat4(1.0f), scene);
}

void ModelStatic::processNode(aiNode* node, glm::mat4 combinedTransform, const aiScene* scene) {
    glm::mat4 thisTransformMtx = castMat4(node->mTransformation);
    combinedTransform *= thisTransformMtx;
    if (VERBOSE_OUTPUT_) {
        cout << "  Node " << node->mName.C_Str() << " has " << node->mNumMeshes << " meshes and " << node->mNumChildren << " children.\n";
        if (thisTransformMtx == glm::mat4(1.0f)) {
            cout << "  Transform: IdentityMtx\n";
        } else {
            cout << "  Transform: " << glm::to_string(thisTransformMtx) << "\n";
        }
    }
    
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {    // Process all meshes in this node.
        meshes_.push_back(processMesh(scene->mMeshes[node->mMeshes[i]], scene));
        meshTransforms_.push_back(combinedTransform);
    }
    for (unsigned int i = 0; i < node->mNumChildren; ++i) {    // Process all child nodes.
        processNode(node->mChildren[i], combinedTransform, scene);
    }
}

Mesh ModelStatic::processMesh(aiMesh* mesh, const aiScene* scene) {
    vector<Mesh::Vertex> vertices;
    vector<unsigned int> indices;
    vector<Mesh::Texture> textures;
    processMeshAttributes<Mesh::Vertex>(mesh, scene, vertices, indices, textures);
    
    return Mesh(move(vertices), move(indices), move(textures));
}
