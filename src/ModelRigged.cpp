#include "ModelRigged.h"
#include "Simulator.h"
#include <cassert>
#include <iostream>
#include <stack>
#include <utility>

ModelRigged::ModelRigged() {
    rootNode_ = nullptr;
}

ModelRigged::ModelRigged(const string& filename) {
    rootNode_ = nullptr;
    loadFile(filename);
}

ModelRigged::~ModelRigged() {
    if (rootNode_ != nullptr) {
        stack<Node*> nodeStack;
        nodeStack.push(rootNode_);
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

void ModelRigged::loadFile(const string& filename) {
    Assimp::Importer importer;
    const aiScene* scene = loadScene(&importer, filename);
    if (scene == nullptr) {
        return;
    }
    meshes_.reserve(scene->mNumMeshes);
    meshTransforms_.reserve(scene->mNumMeshes);
    numNodes_ = 0;
    unordered_map<string, unsigned int> boneMapping;
    globalInverseMtx_ = glm::inverse(castMat4(scene->mRootNode->mTransformation));
    rootNode_ = processNode(nullptr, scene->mRootNode, glm::mat4(1.0f), scene, boneMapping);
    
    unordered_map<string, unsigned int> nodeMapping;    // Traverse node hierarchy again to set bone indices.
    stack<Node*> nodeStack;
    nodeStack.push(rootNode_);
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
    
    animations_.reserve(scene->mNumAnimations);
    for (unsigned int i = 0; i < scene->mNumAnimations; ++i) {
        animations_.emplace_back(string(scene->mAnimations[i]->mName.C_Str()), scene->mAnimations[i]->mDuration, (scene->mAnimations[i]->mTicksPerSecond != 0.0 ? scene->mAnimations[i]->mTicksPerSecond : 20.0));
        
        animations_.back().channels_.resize(numNodes_);
        for (unsigned int j = 0; j < scene->mAnimations[i]->mNumChannels; ++j) {
            const aiNodeAnim* nodeAnim = scene->mAnimations[i]->mChannels[j];
            auto findResult = nodeMapping.find(nodeAnim->mNodeName.C_Str());
            if (findResult == nodeMapping.end()) {
                cout << "Warn: Animation contains a channel item with no corresponding node name.\n";
            } else {
                Animation::Channel* channel = &animations_.back().channels_[findResult->second];
                
                channel->translationKeys.reserve(nodeAnim->mNumPositionKeys);
                for (unsigned int k = 0; k < nodeAnim->mNumPositionKeys; ++k) {
                    channel->translationKeys.emplace_back(castVec3(nodeAnim->mPositionKeys[k].mValue), nodeAnim->mPositionKeys[k].mTime);
                }
                channel->rotationKeys.reserve(nodeAnim->mNumRotationKeys);
                for (unsigned int k = 0; k < nodeAnim->mNumRotationKeys; ++k) {
                    channel->rotationKeys.emplace_back(castQuat(nodeAnim->mRotationKeys[k].mValue), nodeAnim->mRotationKeys[k].mTime);
                }
                channel->scalingKeys.reserve(nodeAnim->mNumScalingKeys);
                for (unsigned int k = 0; k < nodeAnim->mNumScalingKeys; ++k) {
                    channel->scalingKeys.emplace_back(castVec3(nodeAnim->mScalingKeys[k].mValue), nodeAnim->mScalingKeys[k].mTime);
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

void ModelRigged::animate(unsigned int animationIndex, double time, vector<glm::mat4>& boneTransforms) {
    boneTransforms.resize(boneOffsetMatrices_.size());
    
    double animationTime = fmod(time * animations_[animationIndex].ticksPerSecond_, animations_[animationIndex].duration_);
    animateNodes(rootNode_, animations_[animationIndex], animationTime, glm::mat4(1.0f), boneTransforms);
}

ModelRigged::Node* ModelRigged::processNode(Node* parent, aiNode* node, glm::mat4 combinedTransform, const aiScene* scene, unordered_map<string, unsigned int>& boneMapping) {
    glm::mat4 thisTransformMtx = castMat4(node->mTransformation);
    combinedTransform *= thisTransformMtx;
    Node* newNode = new Node(parent, string(node->mName.C_Str()), numNodes_, thisTransformMtx);
    ++numNodes_;
    if (VERBOSE_OUTPUT_) {
        cout << "  Node " << node->mName.C_Str() << " has " << node->mNumMeshes << " meshes and " << node->mNumChildren << " children.\n";
        if (thisTransformMtx == glm::mat4(1.0f)) {
            cout << "  Transform: IdentityMtx\n";
        } else {
            cout << "  Transform: " << glm::to_string(thisTransformMtx) << "\n";
        }
    }
    
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {    // Process all meshes in this node.
        meshes_.push_back(processMesh(scene->mMeshes[node->mMeshes[i]], scene, boneMapping));
        meshTransforms_.push_back(combinedTransform);
    }
    for (unsigned int i = 0; i < node->mNumChildren; ++i) {    // Process all child nodes.
        newNode->children.push_back(processNode(newNode, node->mChildren[i], combinedTransform, scene, boneMapping));
    }
    return newNode;
}

Mesh ModelRigged::processMesh(aiMesh* mesh, const aiScene* scene, unordered_map<string, unsigned int>& boneMapping) {
    vector<Mesh::VertexBone> vertices;
    vector<unsigned int> indices;
    vector<Mesh::Texture> textures;
    processMeshAttributes<Mesh::VertexBone>(mesh, scene, vertices, indices, textures);
    
    for (unsigned int i = 0; i < mesh->mNumBones; ++i) {
        if (VERBOSE_OUTPUT_) {
            cout << "      Bone " << mesh->mBones[i]->mName.C_Str() << " has " << mesh->mBones[i]->mNumWeights << " weights.\n";
        }
        unsigned int boneID;
        string boneName(mesh->mBones[i]->mName.C_Str());
        auto findResult = boneMapping.find(boneName);
        if (findResult == boneMapping.end()) {
            boneID = static_cast<unsigned int>(boneMapping.size());
            boneMapping[boneName] = boneID;
            boneOffsetMatrices_.push_back(castMat4(mesh->mBones[i]->mOffsetMatrix));
        } else {
            boneID = findResult->second;
        }
        
        for (unsigned int j = 0; j < mesh->mBones[i]->mNumWeights; ++j) {
            vertices[mesh->mBones[i]->mWeights[j].mVertexId].addBone(boneID, mesh->mBones[i]->mWeights[j].mWeight);
        }
    }
    
    return Mesh(move(vertices), move(indices), move(textures));
}

void ModelRigged::animateNodes(const Node* node, const Animation& animation, double animationTime, glm::mat4 combinedTransform, vector<glm::mat4>& boneTransforms) const {
    glm::mat4 nodeTransform = node->transform;
    if (animation.channels_[node->id].translationKeys.size() > 0) {    // Check if this node has an animation.
        nodeTransform = animation.calcChannelTransform(node->id, animationTime);
    }
    
    combinedTransform *= nodeTransform;
    
    if (node->boneIndex != -1) {
        boneTransforms[node->boneIndex] = globalInverseMtx_ * combinedTransform * boneOffsetMatrices_[node->boneIndex];
    }
    
    for (unsigned int i = 0; i < node->children.size(); ++i) {
        animateNodes(node->children[i], animation, animationTime, combinedTransform, boneTransforms);
    }
}
