#include "ModelRigged.h"
#include <glm/gtx/quaternion.hpp>
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

ModelRigged::Node* ModelRigged::getRootNode() const {
    return rootNode_;
}

unsigned int ModelRigged::getNumNodes() const {
    return numNodes_;
}

const glm::mat4& ModelRigged::getGlobalInverseMtx() const {
    return globalInverseMtx_;
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
    unordered_map<string, uint8_t> boneMapping;
    rootNode_ = processNode(nullptr, scene->mRootNode, glm::mat4(1.0f), scene, boneMapping);
    
    unordered_map<string, unsigned int> nodeMapping;    // Traverse node hierarchy again to set bone indices.
    stack<Node*> nodeStack;
    nodeStack.push(rootNode_);
    stack<glm::mat4> combinedTransformStack;    // Keep another stack of combined transforms to find the globalInverseMtx_.
    combinedTransformStack.push(rootNode_->transform);
    bool foundFirstBone = false;
    while (!nodeStack.empty()) {
        Node* first = nodeStack.top();
        nodeStack.pop();
        glm::mat4 firstCombinedTransform;
        if (!foundFirstBone) {
            firstCombinedTransform = combinedTransformStack.top();
            combinedTransformStack.pop();
        }
        nodeMapping[first->name] = first->id;
        auto findResult = boneMapping.find(first->name);
        if (findResult != boneMapping.end()) {
            first->boneIndex = findResult->second;
            if (!foundFirstBone) {    // When the first bone is found, compute the globalInverseMtx_ as the matrix that can be multiplied by (combinedTransforms * boneOffsetMatrix) to get the identity matrix.
                globalInverseMtx_ = glm::inverse(firstCombinedTransform * boneOffsetMatrices_[first->boneIndex]);    // This step is key because for whatever reason animation key frames and boneOffsetMatrices_ are relative to the first bone, not the model origin!
                foundFirstBone = true;
            }
        }
        
        for (Node* n : first->children) {
            nodeStack.push(n);
            if (!foundFirstBone) {
                combinedTransformStack.push(firstCombinedTransform * n->transform);
            }
        }
    }
    
    animations_.reserve(scene->mNumAnimations);
    for (unsigned int i = 0; i < scene->mNumAnimations; ++i) {    // Load all animations into the model.
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

void ModelRigged::ragdoll(map<int, DynamicBone>& dynamicBones, vector<glm::mat4>& boneTransforms) const {
    assert(boneTransforms.size() == boneOffsetMatrices_.size());
    
    ragdollNodes(rootNode_, dynamicBones, glm::mat4(1.0f), boneTransforms);
}

void ModelRigged::animate(unsigned int animationIndex, double time, vector<glm::mat4>& boneTransforms) const {
    assert(boneTransforms.size() == boneOffsetMatrices_.size());
    
    double animationTime = fmod(time * animations_[animationIndex].ticksPerSecond_, animations_[animationIndex].duration_);
    animateNodes(rootNode_, animations_[animationIndex], animationTime, globalInverseMtx_, boneTransforms);
}

ModelRigged::Node* ModelRigged::processNode(Node* parent, aiNode* node, glm::mat4 combinedTransform, const aiScene* scene, unordered_map<string, uint8_t>& boneMapping) {
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

Mesh ModelRigged::processMesh(aiMesh* mesh, const aiScene* scene, unordered_map<string, uint8_t>& boneMapping) {
    vector<Mesh::VertexBone> vertices;
    vector<unsigned int> indices;
    vector<Mesh::Texture> textures;
    processMeshAttributes<Mesh::VertexBone>(mesh, scene, vertices, indices, textures);
    
    for (unsigned int i = 0; i < mesh->mNumBones; ++i) {    // Iterate through all bones that effect this mesh (may include bones that have zero weights and do not effect the mesh).
        if (VERBOSE_OUTPUT_) {
            cout << "      Bone " << mesh->mBones[i]->mName.C_Str() << " has " << mesh->mBones[i]->mNumWeights << " weights.\n";
        }
        uint8_t boneID;
        string boneName(mesh->mBones[i]->mName.C_Str());
        auto findResult = boneMapping.find(boneName);
        if (findResult == boneMapping.end()) {    // Found a new bone, add it to the look-up table and add an offset matrix for it.
            boneID = static_cast<uint8_t>(boneMapping.size());
            boneMapping[boneName] = boneID;
            boneOffsetMatrices_.push_back(castMat4(mesh->mBones[i]->mOffsetMatrix));
        } else {    // Found an existing bone.
            boneID = findResult->second;
        }
        
        for (unsigned int j = 0; j < mesh->mBones[i]->mNumWeights; ++j) {
            vertices[mesh->mBones[i]->mWeights[j].mVertexId].addBone(boneID, mesh->mBones[i]->mWeights[j].mWeight);
        }
    }
    
    return Mesh(move(vertices), move(indices), move(textures));
}

void ModelRigged::ragdollNodes(const Node* node, map<int, DynamicBone>& dynamicBones, glm::mat4 combinedTransform, vector<glm::mat4>& boneTransforms) const {
    auto findResult = dynamicBones.find(node->boneIndex);
    if (findResult != dynamicBones.end()) {
        /*DynamicBone& bone = findResult->second;
        glm::mat4 nodeTransform = glm::translate(glm::mat4(1.0f), bone.linearVel);
        nodeTransform *= glm::mat4_cast(bone.angularVel);
        bone.linearVel += bone.linearAcc;
        bone.angularVel *= bone.angularAcc;
        
        combinedTransform *= glm::inverse(boneOffsetMatrices_[node->boneIndex]) * nodeTransform * boneOffsetMatrices_[node->boneIndex];
        boneTransforms[node->boneIndex] *= combinedTransform;
        
        /*glm::vec3 currentDir = glm::mat3(boneTransforms[node->boneIndex]) * glm::vec3(0.0f, 0.0f, 1.0f);
        cout << "delta: " << glm::length(currentDir - glm::vec3(0.0f, 0.0f, 1.0f)) << "\n";
        if (true) {
            glm::quat restoringQuat = findRotationBetweenVectors(glm::vec3(0.0f, 0.0f, 1.0f), currentDir);
            cout << "restoring:  " << glm::to_string(restoringQuat) << "\n";
            restoringQuat = glm::angleAxis(glm::angle(restoringQuat) * 0.01f, glm::axis(restoringQuat));
            bone.angularAcc = restoringQuat;
            cout << "restoring2: " << glm::to_string(restoringQuat) << "\n";
        }*/
        
        //glm::vec3 currentPos = glm::vec3(boneOffsetMatrices_[node->boneIndex] * boneTransforms[node->boneIndex] * glm::inverse(boneOffsetMatrices_[node->boneIndex]) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        //bone.linearAcc = -0.02f * bone.linearVel - 0.002f * currentPos;
        
        //cout << "currentPos = " << glm::to_string(currentPos) << "\n";
        
        DynamicBone& bone = findResult->second;
        glm::vec3 currentPos = glm::vec3(boneOffsetMatrices_[node->boneIndex] * boneTransforms[node->boneIndex] * glm::inverse(boneOffsetMatrices_[node->boneIndex]) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        bone.springMotion.updateMotion(&currentPos, &bone.linearVel, glm::vec3(0.0f, 0.0f, 0.0f));
        glm::mat4 nodeTransform = glm::translate(glm::mat4(1.0f), currentPos);
        
        combinedTransform *= glm::inverse(boneOffsetMatrices_[node->boneIndex]) * nodeTransform * boneOffsetMatrices_[node->boneIndex];
        boneTransforms[node->boneIndex] = combinedTransform;
        
        currentPos = glm::vec3(boneOffsetMatrices_[node->boneIndex] * boneTransforms[node->boneIndex] * glm::inverse(boneOffsetMatrices_[node->boneIndex]) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        cout << "currentPos = " << glm::to_string(currentPos) << "\n";
    }
    
    for (unsigned int i = 0; i < node->children.size(); ++i) {
        ragdollNodes(node->children[i], dynamicBones, combinedTransform, boneTransforms);
    }
}

void ModelRigged::animateNodes(const Node* node, const Animation& animation, double animationTime, glm::mat4 combinedTransform, vector<glm::mat4>& boneTransforms) const {
    //cout << "animateNodes() on " << node->name << " with id " << node->id << " and boneIndex " << node->boneIndex << "\n";
    glm::mat4 nodeTransform = node->transform;
    if (animation.channels_[node->id].translationKeys.size() > 0) {    // Check if this node has an animation.
        //cout << "  adding channel transform.\n";
        nodeTransform = animation.calcChannelTransform(node->id, animationTime);
    }
    
    combinedTransform *= nodeTransform;
    
    if (node->boneIndex != -1) {
        //cout << "  setting bone transform.\n";
        boneTransforms[node->boneIndex] = combinedTransform * boneOffsetMatrices_[node->boneIndex];
    }
    
    //cout << "    evaluating " << node->children.size() << " children.\n";
    for (unsigned int i = 0; i < node->children.size(); ++i) {
        animateNodes(node->children[i], animation, animationTime, combinedTransform, boneTransforms);
    }
}

const ModelRigged::Node* ModelRigged::findNode(const string& nodeName) const {
    stack<Node*> nodeStack;
    nodeStack.push(rootNode_);
    while (!nodeStack.empty()) {
        Node* first = nodeStack.top();
        nodeStack.pop();
        
        if (first->name == nodeName) {
            return first;
        }
        
        for (Node* n : first->children) {
            nodeStack.push(n);
        }
    }
    
    cout << "Error: Unable to find node with name " << nodeName << ".\n";
    return nullptr;
}

glm::quat ModelRigged::findRotationBetweenVectors(glm::vec3 source, glm::vec3 destination) const {
    source = glm::normalize(source);
    destination = glm::normalize(destination);
    float cosTheta = glm::dot(source, destination);
    
    if (cosTheta < -1.0f + 0.001f) {    // Special case where vectors are in opposite direction.
        glm::vec3 axis = glm::cross(source, glm::vec3(1.0f, 0.0f, 0.0f));
        if (glm::length(axis) < 0.01f) {
            axis = glm::cross(source, glm::vec3(0.0f, 1.0f, 0.0f));
        }
        return glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::pi<float>(), axis);
    }
    
    glm::vec3 axis = glm::cross(source, destination);
    float s = sqrt((1.0f + cosTheta) * 2.0f);
    return glm::quat(s / 2.0f, axis.x / s, axis.y / s, axis.z / s);
}
