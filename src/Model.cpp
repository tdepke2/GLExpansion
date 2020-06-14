#include "Model.h"
#include "Simulator.h"
#include <cassert>
#include <iostream>
#include <utility>

Model::Model() {}

Model::Model(const string& filename, const glm::mat4& transformMtx) {
    loadFile(filename, transformMtx);
}

void Model::loadFile(const string& filename, const glm::mat4& transformMtx) {
    assert(meshes.empty());
    if (VERBOSE_OUTPUT) {
        cout << "Loading model \"" << filename << "\".\n";
    }
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filename, aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_GenNormals);
    
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
    _processNode(scene->mRootNode, scene, transformMtx);
}

void Model::applyInstanceBuffer(unsigned int startIndex) const {
    for (size_t i = 0; i < meshes.size(); ++i) {
        meshes[i].applyInstanceBuffer(startIndex);
    }
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

void Model::drawInstanced(unsigned int count) const {
    for (size_t i = 0; i < meshes.size(); ++i) {
        meshes[i].drawInstanced(count);
    }
}

void Model::drawInstanced(const Shader& shader, unsigned int count) const {
    for (size_t i = 0; i < meshes.size(); ++i) {
        meshes[i].drawInstanced(shader, count);
    }
}

void Model::_processNode(aiNode* node, const aiScene* scene, const glm::mat4& transformMtx) {
    glm::mat4 thisTransformMtx = glm::transpose(glm::make_mat4(&node->mTransformation.a1));
    if (VERBOSE_OUTPUT) {
        cout << "  Node " << node->mName.C_Str() << " has " << node->mNumMeshes << " meshes and " << node->mNumChildren << " children.\n";
        cout << "  Transform: " << glm::to_string(thisTransformMtx) << "\n";
    }
    
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {    // Process all meshes in this node.
        meshes.push_back(_processMesh(scene->mMeshes[node->mMeshes[i]], scene, thisTransformMtx * transformMtx));
    }
    for (unsigned int i = 0; i < node->mNumChildren; ++i) {    // Process all child nodes.
        _processNode(node->mChildren[i], scene, thisTransformMtx * transformMtx);
    }
}

Mesh Model::_processMesh(aiMesh* mesh, const aiScene* scene, const glm::mat4& transformMtx) {
    if (VERBOSE_OUTPUT) {
        cout << "    Mesh " << mesh->mName.C_Str() << " has " << mesh->mNumBones << " bones, " << mesh->mNumFaces << " faces, and " << mesh->mNumVertices << " vertices.\n";
    }
    vector<Mesh::Vertex> vertices;
    vertices.reserve(mesh->mNumVertices);
    glm::mat3 normalMtx;
    bool applyTransform = false;
    if (transformMtx != glm::mat4(1.0f)) {
        normalMtx = glm::mat3(glm::transpose(glm::inverse(transformMtx)));
        applyTransform = true;
    }
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        glm::vec4 vPosition(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1.0f);
        glm::vec3 vNormal(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        glm::vec2 vTexCoords;
        if (mesh->mTextureCoords[0]) {
            vTexCoords.x = mesh->mTextureCoords[0][i].x;
            vTexCoords.y = mesh->mTextureCoords[0][i].y;
        } else {
            vTexCoords.x = 0.0f;
            vTexCoords.y = 0.0f;
        }
        glm::vec3 vTangent(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
        glm::vec3 vBitangent(-mesh->mBitangents[i].x, -mesh->mBitangents[i].y, -mesh->mBitangents[i].z);
        
        if (applyTransform) {    // Uhh idk how to transform tangent/bitangent, need to check this ####################################################################################################################
            //vPosition = transformMtx * vPosition;
            //vNormal = normalMtx * vNormal;
        }
        
        vertices.emplace_back(vPosition, vNormal, vTexCoords, vTangent, vBitangent);
        //cout << "      vP = [" << vPosition.x << ", " << vPosition.y << ", " << vPosition.z << "], vN = [" << vNormal.x << ", " << vNormal.y << ", " << vNormal.z << "], vTC = [" << vTexCoords.s << ", " << vTexCoords.t << "], vT = [" << vTangent.x << ", " << vTangent.y << ", " << vTangent.z << "], vB = [" << vBitangent.x << ", " << vBitangent.y << ", " << vBitangent.z << "]\n";
    }
    
    vector<unsigned int> indices;
    indices.reserve(mesh->mNumFaces * 3);
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; ++j) {
            indices.push_back(mesh->mFaces[i].mIndices[j]);
        }
    }
    
    vector<Mesh::Texture> textures;
    _loadMaterialTextures(scene->mMaterials[mesh->mMaterialIndex], aiTextureType_DIFFUSE, "material.texDiffuse", 0, textures);
    _loadMaterialTextures(scene->mMaterials[mesh->mMaterialIndex], aiTextureType_SPECULAR, "material.texSpecular", 1, textures);
    _loadMaterialTextures(scene->mMaterials[mesh->mMaterialIndex], aiTextureType_HEIGHT, "material.texNormal", 2, textures);
    
    return Mesh(move(vertices), move(indices), move(textures));
}

void Model::_loadMaterialTextures(aiMaterial* material, aiTextureType type, const string& uniformName, unsigned int index, vector<Mesh::Texture>& textures) {
    if (VERBOSE_OUTPUT) {
        cout << "      Material " << uniformName << " has " << material->GetTextureCount(type) << " textures:\n";
    }
    //for (unsigned int i = 0; i < material->GetTextureCount(type); ++i) {    // Right now, only the first texture is used :/ #####################################################################################################
    if (material->GetTextureCount(type) > 0) {
        aiString str;
        material->GetTexture(type, 0, &str);
        if (VERBOSE_OUTPUT) {
            cout << "      \"" << str.C_Str() << "\"\n";
        }
        textures.emplace_back(Simulator::loadTexture(directoryPath + "/" + string(str.C_Str()), type == aiTextureType_DIFFUSE), index);
    }
}
