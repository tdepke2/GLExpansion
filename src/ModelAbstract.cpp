#include "ModelAbstract.h"
#include "Renderer.h"
#include "Shader.h"
#include <cassert>
#include <iostream>

ModelAbstract::ModelAbstract() {}

ModelAbstract::~ModelAbstract() {}

void ModelAbstract::applyInstanceBuffer(unsigned int startIndex) const {
    for (const Mesh& m : meshes_) {
        m.applyInstanceBuffer(startIndex);
    }
}

void ModelAbstract::draw(const Shader& shader, const glm::mat4& modelMtx) const {
    for (size_t i = 0; i < meshes_.size(); ++i) {
        meshes_[i].draw(shader, modelMtx * meshTransforms_[i]);
    }
}

void ModelAbstract::drawGeometry() const {
    for (const Mesh& m : meshes_) {
        m.drawGeometry();
    }
}

void ModelAbstract::drawGeometry(const Shader& shader, const glm::mat4& modelMtx) const {
    for (size_t i = 0; i < meshes_.size(); ++i) {
        meshes_[i].drawGeometry(shader, modelMtx * meshTransforms_[i]);
    }
}

void ModelAbstract::drawInstanced(const Shader& shader, unsigned int count) const {
    for (const Mesh& m : meshes_) {
        m.drawInstanced(shader, count);
    }
}

void ModelAbstract::drawGeometryInstanced(const Shader& shader, unsigned int count) const {
    for (const Mesh& m : meshes_) {
        m.drawGeometryInstanced(shader, count);
    }
}

const aiScene* ModelAbstract::loadScene(Assimp::Importer* importer, const string& filename) {
    assert(meshes_.empty());
    if (VERBOSE_OUTPUT_) {
        cout << "Loading model \"" << filename << "\".\n";
    }
    const aiScene* scene = importer->ReadFile(filename, aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_GenSmoothNormals);
    
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        cout << "Failed to load model file \"" << filename << "\": " << importer->GetErrorString() << "\n";
        return nullptr;
    }
    directoryPath_ = filename.substr(0, filename.find_last_of('/'));
    
    if (VERBOSE_OUTPUT_) {
        cout << "  Number of animations: " << scene->mNumAnimations << "\n";
        cout << "  Number of cameras:    " << scene->mNumCameras << "\n";
        cout << "  Number of lights:     " << scene->mNumLights << "\n";
        cout << "  Number of materials:  " << scene->mNumMaterials << "\n";
        cout << "  Number of meshes:     " << scene->mNumMeshes << "\n";
        cout << "  Number of textures:   " << scene->mNumTextures << "\n";
    }
    
    return scene;
}

void ModelAbstract::loadMaterialTextures(aiMaterial* material, aiTextureType type, const string& uniformName, unsigned int index, vector<Mesh::Texture>& textures) {
    if (VERBOSE_OUTPUT_) {
        cout << "      Material " << uniformName << " has " << material->GetTextureCount(type) << " textures:\n";
    }
    //for (unsigned int i = 0; i < material->GetTextureCount(type); ++i) {    // Right now, only the first texture is used :/ #####################################################################################################
    if (material->GetTextureCount(type) > 0) {
        aiString str;
        material->GetTexture(type, 0, &str);
        if (VERBOSE_OUTPUT_) {
            cout << "      \"" << str.C_Str() << "\"\n";
        }
        textures.emplace_back(Renderer::loadTexture(directoryPath_ + "/" + string(str.C_Str()), type == aiTextureType_DIFFUSE), index);
    }
}

template<typename V>
void ModelAbstract::processMeshAttributes(aiMesh* mesh, const aiScene* scene, vector<V>& vertices, vector<unsigned int>& indices, vector<Mesh::Texture>& textures) {
    if (VERBOSE_OUTPUT_) {
        cout << "    Mesh " << mesh->mName.C_Str() << " has " << mesh->mNumBones << " bones, " << mesh->mNumFaces << " faces, and " << mesh->mNumVertices << " vertices.\n";
    }
    vertices.reserve(mesh->mNumVertices);
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        glm::vec4 vPosition(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1.0f);
        glm::vec3 vNormal(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        glm::vec2 vTexCoords;
        glm::vec3 vTangent, vBitangent;
        if (mesh->mTextureCoords[0]) {
            vTexCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
            vTangent = glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
            vBitangent = glm::vec3(-mesh->mBitangents[i].x, -mesh->mBitangents[i].y, -mesh->mBitangents[i].z);
        } else {
            vTexCoords = glm::vec2(0.0f);
            vTangent = glm::vec3(0.0f);
            vBitangent = glm::vec3(0.0f);
        }
        
        vertices.emplace_back(vPosition, vNormal, vTexCoords, vTangent, vBitangent);
        //cout << "      vP = [" << vPosition.x << ", " << vPosition.y << ", " << vPosition.z << "], vN = [" << vNormal.x << ", " << vNormal.y << ", " << vNormal.z << "], vTC = [" << vTexCoords.s << ", " << vTexCoords.t << "], vT = [" << vTangent.x << ", " << vTangent.y << ", " << vTangent.z << "], vB = [" << vBitangent.x << ", " << vBitangent.y << ", " << vBitangent.z << "]\n";
    }
    
    indices.reserve(mesh->mNumFaces * 3);
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; ++j) {
            indices.push_back(mesh->mFaces[i].mIndices[j]);
        }
    }
    
    // No need to pass vector reference to loadMaterialTextures anymore. ################################################################################################################
    loadMaterialTextures(scene->mMaterials[mesh->mMaterialIndex], aiTextureType_DIFFUSE, "material.texDiffuse", 0, textures);
    loadMaterialTextures(scene->mMaterials[mesh->mMaterialIndex], aiTextureType_SPECULAR, "material.texSpecular", 1, textures);
    loadMaterialTextures(scene->mMaterials[mesh->mMaterialIndex], aiTextureType_NORMALS, "material.texNormal", 2, textures);
    loadMaterialTextures(scene->mMaterials[mesh->mMaterialIndex], aiTextureType_DISPLACEMENT, "material.texDisplacement", 3, textures);
}
template void ModelAbstract::processMeshAttributes<Mesh::Vertex>(aiMesh* mesh, const aiScene* scene, vector<Mesh::Vertex>& vertices, vector<unsigned int>& indices, vector<Mesh::Texture>& textures);
template void ModelAbstract::processMeshAttributes<Mesh::VertexBone>(aiMesh* mesh, const aiScene* scene, vector<Mesh::VertexBone>& vertices, vector<unsigned int>& indices, vector<Mesh::Texture>& textures);
