#include "Animation.h"
#include "ModelRigged.h"
#include <glm/gtx/quaternion.hpp>
#include <cassert>
#include <iostream>
#include <stack>
#include <utility>

glm::quat ModelRigged::findRotationBetweenVectors(glm::vec3 source, glm::vec3 destination) {
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

ModelRigged::ModelRigged() {
    rootNode_ = nullptr;
}

ModelRigged::ModelRigged(const string& filename, unordered_map<string, Animation>* animations) {
    rootNode_ = nullptr;
    loadFile(filename, animations);
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

const glm::mat4& ModelRigged::getArmatureRootInv() const {
    return armatureRootInv_;
}

void ModelRigged::setArmatureRootInv(const glm::mat4& armatureRootInv) {
    armatureRootInv_ = armatureRootInv;
}

void ModelRigged::loadFile(const string& filename, unordered_map<string, Animation>* animations) {
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
    
    stack<Node*> nodeStack;    // Traverse node hierarchy again to set bone indices.
    nodeStack.push(rootNode_);
    stack<glm::mat4> combinedTransformStack;    // Keep another stack of combined transforms to find the armatureRootInv_.
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
        auto findResult = boneMapping.find(first->name);
        if (findResult != boneMapping.end()) {
            first->boneIndex = findResult->second;
            if (!foundFirstBone) {
                // When the first bone is found, compute the armatureRootInv_ as the inverse of the armature root transform.
                // In theory, this can be multiplied by (combinedTransforms * boneOffsetMatrix) to get the identity matrix when the animation matches bind pose.
                // Some models don't work properly with this, so the setArmatureRootInv() can be used as an override.
                // This step is key because for whatever reason, animation key frames and boneOffsetMatrices_ are relative to the first bone, not the model origin!
                armatureRootInv_ = glm::inverse(firstCombinedTransform * glm::inverse(first->transform));
                if (VERBOSE_OUTPUT_) {
                    cout << "Transform of the armature root computed as " << glm::to_string(firstCombinedTransform * glm::inverse(first->transform)) << ".\n";
                }
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
    
    if (animations != nullptr) {
        for (unsigned int i = 0; i < scene->mNumAnimations; ++i) {    // Load all animations of the model.
            auto insertResult = animations->insert({string(scene->mAnimations[i]->mName.C_Str()), Animation(scene, i)});
            if (!insertResult.second) {
                cout << "Error: Found animation with the same name as an existing one.\n";
            }
        }
    }
}

void ModelRigged::ragdoll(const glm::mat4& modelMtx, map<int, DynamicBone>& dynamicBones, vector<glm::mat4>& boneTransforms) const {
    assert(boneTransforms.size() == boneOffsetMatrices_.size());
    
    ragdollNodes(rootNode_, modelMtx, dynamicBones, armatureRootInv_, boneTransforms);
}

void ModelRigged::animate(const Animation& animation, double time, vector<glm::mat4>& boneTransforms) const {
    assert(boneTransforms.size() == boneOffsetMatrices_.size());
    
    double animationTime = fmod(time * animation.ticksPerSecond_, animation.duration_);
    animateNodes(rootNode_, animation, animationTime, armatureRootInv_, boneTransforms);
}

void ModelRigged::animateWithDynamics(const Animation& animation, double time, const glm::mat4& modelMtx, map<int, DynamicBone>& dynamicBones, vector<glm::mat4>& boneTransforms) const {
    assert(boneTransforms.size() == boneOffsetMatrices_.size());
    
    double animationTime = fmod(time * animation.ticksPerSecond_, animation.duration_);
    animateNodesWithDynamics(rootNode_, animation, animationTime, modelMtx, dynamicBones, armatureRootInv_, boneTransforms);
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

void ModelRigged::ragdollNodes(const Node* node, const glm::mat4& modelMtx, map<int, DynamicBone>& dynamicBones, glm::mat4 combinedTransform, vector<glm::mat4>& boneTransforms) const {
    glm::mat4 nodeTransform = node->transform;
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
        
        //glm::vec3 parentPos = glm::vec3(modelMtx * glm::inverse(armatureRootInv_) * boneTransforms[node->boneIndex] * glm::inverse(boneOffsetMatrices_[node->boneIndex]) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        //cout << "parentPos = " << glm::to_string(parentPos) << "\n";
        
        // Applying physics to a dynamic bone overwrites any keyframes associated with the bone.
        
        DynamicBone& bone = findResult->second;
        glm::vec3 centerOfMassOffset(0.0f, 1.0f, 0.0f);    // this vector should be provided from DynamicBone
        glm::vec3 equilibriumPos    = glm::vec3(modelMtx * glm::inverse(armatureRootInv_) * combinedTransform * node->transform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));    // this should factor in combinedTransform to be relative to parent bone.
        glm::vec3 equilibriumPosCOM = glm::vec3(modelMtx * glm::inverse(armatureRootInv_) * combinedTransform * node->transform * glm::vec4(centerOfMassOffset, 1.0f));
        //cout << "equilibriumPos = " << glm::to_string(equilibriumPos) << "\n";    // equilibriumPos = vec3(-20.056000, 1.184000, -11.968000)
        glm::vec3 test              = glm::vec3(modelMtx * glm::inverse(armatureRootInv_) * glm::inverse(boneOffsetMatrices_[node->boneIndex]) * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));
        bone.springMotion.updateMotion(&bone.lastPosition, &bone.linearVel, equilibriumPos);    // consider using the difference of bone.lastPosition and equilibriumPos as the real position (to account for precision)
        //bone.lastPosition = equilibriumPos;
        bone.springMotion.updateMotion(&bone.lastPositionCOM, &bone.linearVelCOM, equilibriumPosCOM);
        //bone.lastPositionCOM = equilibriumPosCOM;
        //float dotProd = glm::dot(glm::vec3(0.0f, 0.0f, 1.0f), bone.lastPositionCOM);
        //glm::quat result = glm::angleAxis(-acos(dotProd) / glm::length(bone.lastPositionCOM), glm::vec3(1.0f, 0.0f, 0.0f));//findRotationBetweenVectors(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(bone.lastPositionCOM.x, bone.lastPositionCOM.y, 0.0f));
        //float dotProd2 = glm::dot(glm::vec3(1.0f, 0.0f, 0.0f), test);
        //glm::quat result2 = glm::angleAxis(acos(dotProd2) / glm::length(test) * (test.z < 0.0f ? 1.0f : -1.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        //glm::quat rotateToCOM = findRotationBetweenVectors(equilibriumPosCOM - equilibriumPos, bone.lastPositionCOM - equilibriumPos);
        //nodeTransform = armatureRootInv_ * glm::inverse(modelMtx) * glm::translate(glm::mat4(1.0f), bone.lastPosition) * glm::mat4_cast(rotateToCOM) * glm::mat4(glm::mat3(modelMtx * node->transform));
        glm::vec3 bonePosLS = glm::vec3(glm::inverse(node->transform) * glm::inverse(combinedTransform) * armatureRootInv_ * glm::inverse(modelMtx) * glm::vec4(bone.lastPosition, 1.0f));
        glm::vec3 equilibriumPosLS = glm::vec3(glm::inverse(node->transform) * glm::inverse(combinedTransform) * armatureRootInv_ * glm::inverse(modelMtx) * glm::vec4(equilibriumPos, 1.0f));
        glm::vec3 equilibriumPosCOMLS = glm::vec3(glm::inverse(node->transform) * glm::inverse(combinedTransform) * armatureRootInv_ * glm::inverse(modelMtx) * glm::vec4(equilibriumPosCOM, 1.0f));
        glm::vec3 bonePosCOMLS = glm::vec3(glm::inverse(node->transform) * glm::inverse(combinedTransform) * armatureRootInv_ * glm::inverse(modelMtx) * glm::vec4(bone.lastPositionCOM, 1.0f));
        glm::quat rotateToCOM = findRotationBetweenVectors(equilibriumPosCOMLS - equilibriumPosLS, bonePosCOMLS - equilibriumPosLS);
        nodeTransform = combinedTransform * node->transform * glm::translate(glm::mat4(1.0f), bonePosLS) * glm::mat4_cast(rotateToCOM);
        //debugVectors_[1] = characterTest_.transform_.getTransform() * glm::inverse(characterTest_.model_.getArmatureRootInv()) * characterTest_.boneTransforms_[node->boneIndex] * glm::inverse(characterTest_.model_.boneOffsetMatrices_[node->boneIndex]);
        
        combinedTransform *= nodeTransform;
        boneTransforms[node->boneIndex] = nodeTransform * boneOffsetMatrices_[node->boneIndex];    // not quite right
        //cout << "boneTransforms[node->boneIndex] = " << glm::to_string(boneTransforms[node->boneIndex]) << "\n";
        
        glm::vec3 currentPos = glm::vec3(boneOffsetMatrices_[node->boneIndex] * boneTransforms[node->boneIndex] * glm::inverse(boneOffsetMatrices_[node->boneIndex]) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        
        //cout << "    currentPos = " << glm::to_string(currentPos) << "\n";
    } else {
        combinedTransform *= nodeTransform;
    }
    
    for (unsigned int i = 0; i < node->children.size(); ++i) {
        ragdollNodes(node->children[i], modelMtx, dynamicBones, combinedTransform, boneTransforms);
    }
}

void ModelRigged::animateNodes(const Node* node, const Animation& animation, double animationTime, glm::mat4 combinedTransform, vector<glm::mat4>& boneTransforms) const {
    //cout << "animateNodes() on " << node->name << " with id " << node->id << " and boneIndex " << node->boneIndex << "\n";
    glm::mat4 nodeTransform = node->transform;
    auto findResult = animation.channels_.find(node->name);
    if (findResult != animation.channels_.end()) {    // Check if this node has an animation.
        //cout << "  adding channel transform.\n";
        nodeTransform = animation.calcChannelTransform(findResult->second, animationTime);
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

void ModelRigged::animateNodesWithDynamics(const Node* node, const Animation& animation, double animationTime, const glm::mat4& modelMtx, map<int, DynamicBone>& dynamicBones, glm::mat4 combinedTransform, vector<glm::mat4>& boneTransforms) const {
    glm::mat4 nodeTransform = node->transform;
    auto findResult = dynamicBones.find(node->boneIndex);
    if (findResult != dynamicBones.end()) {    // Check if this is a dynamic bone, and override its transforms and any keyframes it may have.
        DynamicBone& bone = findResult->second;
        glm::vec3 centerOfMassOffset(0.0f, 1.0f, 0.0f);    // this vector should be provided from DynamicBone
        glm::vec3 equilibriumPos    = glm::vec3(modelMtx * glm::inverse(armatureRootInv_) * combinedTransform * node->transform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        glm::vec3 equilibriumPosCOM = glm::vec3(modelMtx * glm::inverse(armatureRootInv_) * combinedTransform * node->transform * glm::vec4(centerOfMassOffset, 1.0f));
        
        bone.springMotion.updateMotion(&bone.lastPosition, &bone.linearVel, equilibriumPos);    // consider using the difference of bone.lastPosition and equilibriumPos as the real position (to account for precision)
        //bone.lastPosition = equilibriumPos;
        bone.springMotion.updateMotion(&bone.lastPositionCOM, &bone.linearVelCOM, equilibriumPosCOM);
        //bone.lastPositionCOM = equilibriumPosCOM;
        
        glm::vec3 bonePosLS = glm::vec3(glm::inverse(node->transform) * glm::inverse(combinedTransform) * armatureRootInv_ * glm::inverse(modelMtx) * glm::vec4(bone.lastPosition, 1.0f));
        glm::vec3 equilibriumPosLS = glm::vec3(glm::inverse(node->transform) * glm::inverse(combinedTransform) * armatureRootInv_ * glm::inverse(modelMtx) * glm::vec4(equilibriumPos, 1.0f));
        glm::vec3 equilibriumPosCOMLS = glm::vec3(glm::inverse(node->transform) * glm::inverse(combinedTransform) * armatureRootInv_ * glm::inverse(modelMtx) * glm::vec4(equilibriumPosCOM, 1.0f));
        glm::vec3 bonePosCOMLS = glm::vec3(glm::inverse(node->transform) * glm::inverse(combinedTransform) * armatureRootInv_ * glm::inverse(modelMtx) * glm::vec4(bone.lastPositionCOM, 1.0f));
        glm::quat rotateToCOM = findRotationBetweenVectors(equilibriumPosCOMLS - equilibriumPosLS, bonePosCOMLS - equilibriumPosLS);
        
        nodeTransform = node->transform * glm::translate(glm::mat4(1.0f), bonePosLS) * glm::mat4_cast(rotateToCOM);
    } else {
        auto findResult2 = animation.channels_.find(node->name);
        if (findResult2 != animation.channels_.end()) {    // Check if this node has an animation.
            nodeTransform = animation.calcChannelTransform(findResult2->second, animationTime);
        }
    }
    
    combinedTransform *= nodeTransform;
    
    if (node->boneIndex != -1) {
        boneTransforms[node->boneIndex] = combinedTransform * boneOffsetMatrices_[node->boneIndex];
    }
    
    for (unsigned int i = 0; i < node->children.size(); ++i) {
        animateNodesWithDynamics(node->children[i], animation, animationTime, modelMtx, dynamicBones, combinedTransform, boneTransforms);
    }
}
