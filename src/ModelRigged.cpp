#include "ModelRigged.h"
#include "Simulator.h"
#include <cassert>
#include <iostream>
#include <stack>
#include <utility>

ModelRigged::ModelRigged() {
    rootNode = nullptr;
}

ModelRigged::ModelRigged(const string& filename, const glm::mat4& transformMtx) {
    rootNode = nullptr;
    loadFile(filename, transformMtx);
}

ModelRigged::~ModelRigged() {
    if (rootNode != nullptr) {
        stack<Node*> nodeStack;
        nodeStack.push(rootNode);
        while (!nodeStack.empty()) {
            Node* first = nodeStack.top();
            nodeStack.pop();
            for (Node* n : first->children) {
                nodeStack.push(n);
            }
            delete first;
        }
    }
}

void ModelRigged::loadFile(const string& filename, const glm::mat4& transformMtx) {
    assert(meshes.empty());
    if (VERBOSE_OUTPUT) {
        cout << "Loading model \"" << filename << "\".\n";
    }
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filename, aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_GenSmoothNormals);// smooth or not? idk ###################################################################
    
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
    numNodes = 0;
    unordered_map<string, unsigned int> boneMapping;
    globalInverseMtx = glm::inverse(_castMat4(scene->mRootNode->mTransformation));
    rootNode = _processNode(nullptr, scene->mRootNode, scene, boneMapping, transformMtx);
    
    unordered_map<string, unsigned int> nodeMapping;    // Traverse node hierarchy again to set bone indices.
    stack<Node*> nodeStack;
    nodeStack.push(rootNode);
    while (!nodeStack.empty()) {
        Node* first = nodeStack.top();
        nodeStack.pop();
        nodeMapping[first->name] = first->id;
        auto findResult = boneMapping.find(first->name);
        if (findResult != boneMapping.end()) {
            first->boneIndex = findResult->second;
        }
        
        for (Node* n : first->children) {
            nodeStack.push(n);
        }
    }
    
    animations.reserve(scene->mNumAnimations);
    for (unsigned int i = 0; i < scene->mNumAnimations; ++i) {
        animations.emplace_back(string(scene->mAnimations[i]->mName.C_Str()), scene->mAnimations[i]->mDuration, (scene->mAnimations[i]->mTicksPerSecond != 0.0 ? scene->mAnimations[i]->mTicksPerSecond : 20.0));
        
        animations.back().channels.resize(numNodes);
        for (unsigned int j = 0; j < scene->mAnimations[i]->mNumChannels; ++j) {
            const aiNodeAnim* nodeAnim = scene->mAnimations[i]->mChannels[j];
            auto findResult = nodeMapping.find(nodeAnim->mNodeName.C_Str());
            if (findResult == nodeMapping.end()) {
                cout << "Warn: Animation contains a channel item with no corresponding node name.\n";
            } else {
                Animation::Channel* channel = &animations.back().channels[findResult->second];
                
                channel->translationKeys.reserve(nodeAnim->mNumPositionKeys);
                for (unsigned int k = 0; k < nodeAnim->mNumPositionKeys; ++k) {
                    channel->translationKeys.emplace_back(_castVec3(nodeAnim->mPositionKeys[k].mValue), nodeAnim->mPositionKeys[k].mTime);
                }
                channel->rotationKeys.reserve(nodeAnim->mNumRotationKeys);
                for (unsigned int k = 0; k < nodeAnim->mNumRotationKeys; ++k) {
                    channel->rotationKeys.emplace_back(_castQuat(nodeAnim->mRotationKeys[k].mValue), nodeAnim->mRotationKeys[k].mTime);
                }
                channel->scalingKeys.reserve(nodeAnim->mNumScalingKeys);
                for (unsigned int k = 0; k < nodeAnim->mNumScalingKeys; ++k) {
                    channel->scalingKeys.emplace_back(_castVec3(nodeAnim->mScalingKeys[k].mValue), nodeAnim->mScalingKeys[k].mTime);
                }
                
                if (nodeAnim->mNumPositionKeys + nodeAnim->mNumRotationKeys + nodeAnim->mNumScalingKeys > 1) {
                    if (nodeAnim->mNumPositionKeys == 0 || nodeAnim->mNumRotationKeys == 0 || nodeAnim->mNumScalingKeys == 0) {
                        cout << "Warn: Animation contains a channel with a missing transform key.\n";
                    }
                }
            }
        }
    }
}

void ModelRigged::animate(const Shader* shader, unsigned int animationIndex, double time, vector<glm::mat4>& boneTransforms) {
    boneTransforms.resize(boneOffsetMatrices.size());
    
    double animationTime = fmod(time * animations[animationIndex].ticksPerSecond, animations[animationIndex].duration);
    _animateNodes(rootNode, &animations[animationIndex], animationTime, glm::mat4(1.0f), boneTransforms);
    
    shader->use();
    shader->setMat4Array("boneTransforms", boneTransforms.size(), boneTransforms.data());
}

void ModelRigged::draw() const {
    for (size_t i = 0; i < meshes.size(); ++i) {
        meshes[i].draw();
    }
}

void ModelRigged::draw(const Shader& shader) const {
    for (size_t i = 0; i < meshes.size(); ++i) {
        meshes[i].draw(shader);
    }
}

ModelRigged::Node* ModelRigged::_processNode(Node* parent, aiNode* node, const aiScene* scene, unordered_map<string, unsigned int>& boneMapping, const glm::mat4& transformMtx) {
    glm::mat4 thisTransformMtx = _castMat4(node->mTransformation);
    Node* newNode = new Node(parent, string(node->mName.C_Str()), numNodes, thisTransformMtx);
    ++numNodes;
    if (VERBOSE_OUTPUT) {
        cout << "  Node " << node->mName.C_Str() << " has " << node->mNumMeshes << " meshes and " << node->mNumChildren << " children.\n";
        cout << "  Transform: " << glm::to_string(thisTransformMtx) << "\n";
    }
    
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {    // Process all meshes in this node.
        meshes.push_back(_processMesh(scene->mMeshes[node->mMeshes[i]], scene, boneMapping, thisTransformMtx * transformMtx));    // pretty sure this transform is backwards #########################################################
    }
    for (unsigned int i = 0; i < node->mNumChildren; ++i) {    // Process all child nodes.
        newNode->children.push_back(_processNode(newNode, node->mChildren[i], scene, boneMapping, thisTransformMtx * transformMtx));
    }
    return newNode;
}

MeshRigged ModelRigged::_processMesh(aiMesh* mesh, const aiScene* scene, unordered_map<string, unsigned int>& boneMapping, const glm::mat4& transformMtx) {
    if (VERBOSE_OUTPUT) {
        cout << "    Mesh " << mesh->mName.C_Str() << " has " << mesh->mNumBones << " bones, " << mesh->mNumFaces << " faces, and " << mesh->mNumVertices << " vertices.\n";
    }
    vector<MeshRigged::Vertex> vertices;
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
    
    for (unsigned int i = 0; i < mesh->mNumBones; ++i) {
        if (VERBOSE_OUTPUT) {
            cout << "      Bone " << mesh->mBones[i]->mName.C_Str() << " has " << mesh->mBones[i]->mNumWeights << " weights.\n";
        }
        unsigned int boneID;
        string boneName(mesh->mBones[i]->mName.C_Str());
        auto findResult = boneMapping.find(boneName);
        if (findResult == boneMapping.end()) {
            boneID = static_cast<unsigned int>(boneMapping.size());
            boneMapping[boneName] = boneID;
            boneOffsetMatrices.push_back(_castMat4(mesh->mBones[i]->mOffsetMatrix));
        } else {
            boneID = findResult->second;
        }
        
        for (unsigned int j = 0; j < mesh->mBones[i]->mNumWeights; ++j) {
            vertices[mesh->mBones[i]->mWeights[j].mVertexId].addBone(boneID, mesh->mBones[i]->mWeights[j].mWeight);
        }
    }
    
    vector<MeshRigged::Texture> textures;    // No need to pass vector reference to _loadMaterialTextures anymore. ################################################################################################################
    _loadMaterialTextures(scene->mMaterials[mesh->mMaterialIndex], aiTextureType_DIFFUSE, "material.texDiffuse", 0, textures);
    _loadMaterialTextures(scene->mMaterials[mesh->mMaterialIndex], aiTextureType_SPECULAR, "material.texSpecular", 1, textures);
    _loadMaterialTextures(scene->mMaterials[mesh->mMaterialIndex], aiTextureType_NORMALS, "material.texNormal", 2, textures);
    _loadMaterialTextures(scene->mMaterials[mesh->mMaterialIndex], aiTextureType_DISPLACEMENT, "material.texDisplacement", 3, textures);
    
    return MeshRigged(move(vertices), move(indices), move(textures));
}

void ModelRigged::_loadMaterialTextures(aiMaterial* material, aiTextureType type, const string& uniformName, unsigned int index, vector<MeshRigged::Texture>& textures) {
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

void ModelRigged::_animateNodes(const Node* node, const Animation* animation, double animationTime, glm::mat4 parentTransform, vector<glm::mat4>& boneTransforms) const {
    glm::mat4 nodeTransform = node->transform;
    if (animation->channels[node->id].translationKeys.size() > 0) {    // Check if this node has an animation.
        nodeTransform = animation->calcChannelTransform(node->id, animationTime);
    }
    
    parentTransform *= nodeTransform;
    
    if (node->boneIndex != -1) {
        boneTransforms[node->boneIndex] = globalInverseMtx * parentTransform * boneOffsetMatrices[node->boneIndex];
    }
    
    for (unsigned int i = 0; i < node->children.size(); ++i) {
        _animateNodes(node->children[i], animation, animationTime, parentTransform, boneTransforms);
    }
}
