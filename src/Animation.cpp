#include "Animation.h"
#include "ModelRigged.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <memory>
#include <stack>
#include <stdexcept>

void Animation::loadFile(const string& filename, unordered_map<string, Animation>* animations, const string& repairFilename, const ModelRigged* referenceModel) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filename, aiProcess_JoinIdenticalVertices | aiProcess_Triangulate);
    
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        cout << "Failed to load model file \"" << filename << "\": " << importer.GetErrorString() << "\n";
        return;
    }
    
    unique_ptr<unordered_map<string, string>> nodeSubstitutesPtr;
    if (repairFilename != "") {
        ifstream inputFile(repairFilename);
        if (!inputFile.is_open()) {
            throw runtime_error("\"" + repairFilename + "\": Unable to open file for reading.");
        }
        
        nodeSubstitutesPtr = make_unique<unordered_map<string, string>>();
        string line;
        int lineNumber = 0;
        try {
            while (getline(inputFile, line)) {
                ++lineNumber;
                if (line.length() == 0 || line[0] == '#') {
                    continue;
                }
                string::size_type firstComma = line.find(',');
                nodeSubstitutesPtr->emplace(line.substr(0, firstComma), line.substr(firstComma + 2));
            }
        } catch (exception& ex) {
            inputFile.close();
            throw runtime_error("\"" + repairFilename + "\" at line " + to_string(lineNumber) + ": " + ex.what());
        }
        inputFile.close();
    }
    
    for (unsigned int i = 0; i < scene->mNumAnimations; ++i) {    // Load all animations of the model.
        auto insertResult = animations->insert({string(scene->mAnimations[i]->mName.C_Str()), Animation(scene, i, nodeSubstitutesPtr.get(), referenceModel)});
        if (!insertResult.second) {
            cout << "Error: Found animation with the same name as an existing one.\n";
        }
    }
}

Animation::Animation(const string& name, double duration, double ticksPerSecond) : name_(name), duration_(duration), ticksPerSecond_(ticksPerSecond) {}

Animation::Animation(const aiScene* scene, unsigned int index, const unordered_map<string, string>* nodeSubstitutes, const ModelRigged* referenceModel) {
    loadFromScene(scene, index, nodeSubstitutes, referenceModel);
}

void Animation::loadFromScene(const aiScene* scene, unsigned int index, const unordered_map<string, string>* nodeSubstitutes, const ModelRigged* referenceModel) {
    name_ = string(scene->mAnimations[index]->mName.C_Str());
    duration_ = scene->mAnimations[index]->mDuration;
    ticksPerSecond_ = (scene->mAnimations[index]->mTicksPerSecond != 0.0 ? scene->mAnimations[index]->mTicksPerSecond : 20.0);
    
    cout << "Loading animation " << name_ << " with duration " << duration_ << "s at " << ticksPerSecond_ << " TPS.\n";
    
    for (unsigned int j = 0; j < scene->mAnimations[index]->mNumChannels; ++j) {
        const aiNodeAnim* nodeAnim = scene->mAnimations[index]->mChannels[j];
        string nodeName = string(nodeAnim->mNodeName.C_Str());
        if (nodeSubstitutes != nullptr) {
            auto findResult = nodeSubstitutes->find(nodeName);
            if (findResult != nodeSubstitutes->end()) {
                nodeName = findResult->second;
            }
        }
        
        cout << "  Channel " << nodeName << " has " << nodeAnim->mNumPositionKeys << " position, " << nodeAnim->mNumRotationKeys << " rotation, and " << nodeAnim->mNumScalingKeys << " scaling keys.\n";
        
        Animation::Channel channel;
        channel.translationKeys.reserve(nodeAnim->mNumPositionKeys);
        for (unsigned int k = 0; k < nodeAnim->mNumPositionKeys; ++k) {
            channel.translationKeys.emplace_back(castVec3(nodeAnim->mPositionKeys[k].mValue), nodeAnim->mPositionKeys[k].mTime);
        }
        channel.rotationKeys.reserve(nodeAnim->mNumRotationKeys);
        for (unsigned int k = 0; k < nodeAnim->mNumRotationKeys; ++k) {
            channel.rotationKeys.emplace_back(castQuat(nodeAnim->mRotationKeys[k].mValue), nodeAnim->mRotationKeys[k].mTime);
        }
        channel.scalingKeys.reserve(nodeAnim->mNumScalingKeys);
        for (unsigned int k = 0; k < nodeAnim->mNumScalingKeys; ++k) {
            channel.scalingKeys.emplace_back(castVec3(nodeAnim->mScalingKeys[k].mValue), nodeAnim->mScalingKeys[k].mTime);
        }
        
        if (nodeAnim->mNumPositionKeys + nodeAnim->mNumRotationKeys + nodeAnim->mNumScalingKeys > 1) {
            if (nodeAnim->mNumPositionKeys == 0 || nodeAnim->mNumRotationKeys == 0 || nodeAnim->mNumScalingKeys == 0) {
                cout << "Warn: Animation contains a channel with a missing transform key.\n";
            }
        }
        
        /*for (unsigned int k = 0; k < nodeAnim->mNumPositionKeys; ++k) {
            channel.translationKeys[k].first *= 0.01;
        }
        if (string(nodeAnim->mNodeName.C_Str()) == "Armature") {
            cout << "/////////////////////////////// FOUND ARMATURE ////////////////////////////////////////////////////.\n";
            for (unsigned int k = 0; k < nodeAnim->mNumScalingKeys; ++k) {
                cout << glm::to_string(channel.scalingKeys[k].first) << "\n";
                channel.scalingKeys[k].first = glm::vec3(1.0f);
            }
            auto insertResult = channels_.insert({"Armature", channel});
            if (!insertResult.second) {
                cout << "Warn: Animation contains a duplicate channel name.\n";
            }
        } else {*/
        
        auto insertResult = channels_.insert({nodeName, channel});
        if (!insertResult.second) {
            cout << "Warn: Animation contains a duplicate channel name.\n";
        }
        
        //}
    }
    
    if (nodeSubstitutes != nullptr) {
        printBoneDiffTest(scene, nodeSubstitutes, referenceModel);
    }
}

glm::mat4 Animation::calcChannelTransform(const Channel& channel, double animationTime) const {
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), interpolateVec3Keys(channel.translationKeys, animationTime));
    transform *= glm::mat4_cast(interpolateQuatKeys(channel.rotationKeys, animationTime));
    transform = glm::scale(transform, interpolateVec3Keys(channel.scalingKeys, animationTime));
    
    return transform;
}

glm::vec3 Animation::interpolateVec3Keys(const vector<pair<glm::vec3, double>>& keys, double animationTime) const {
    if (keys.size() == 1) {
        return keys[0].first;
    }
    unsigned int i = 0;
    while (i < keys.size() - 1) {
        if (animationTime < keys[i + 1].second) {
            break;
        }
        ++i;
    }
    if (i >= keys.size() - 1) {
        assert(false);
    }
    
    float t = static_cast<float>((animationTime - keys[i].second) / (keys[i + 1].second - keys[i].second));
    return glm::mix(keys[i].first, keys[i + 1].first, t);
}

glm::quat Animation::interpolateQuatKeys(const vector<pair<glm::quat, double>>& keys, double animationTime) const {
    if (keys.size() == 1) {
        return keys[0].first;
    }
    unsigned int i = 0;
    while (i < keys.size() - 1) {
        if (animationTime < keys[i + 1].second) {
            break;
        }
        ++i;
    }
    if (i >= keys.size() - 1) {
        assert(false);
    }
    
    float t = static_cast<float>((animationTime - keys[i].second) / (keys[i + 1].second - keys[i].second));
    return glm::slerp(keys[i].first, keys[i + 1].first, t);
}

void Animation::printBoneDiffTest(const aiScene* scene, const unordered_map<string, string>* nodeSubstitutes, const ModelRigged* referenceModel) {
    cout << "/////////////////////// printBoneDiffTest //////////////////////////////\n";
    
    unordered_map<string, pair<const aiNode*, glm::mat4>> mappedNodes;
    
    stack<const aiNode*> nodeStack;
    nodeStack.push(scene->mRootNode);
    stack<glm::mat4> transformStack;
    transformStack.push(castMat4(scene->mRootNode->mTransformation));
    while (!nodeStack.empty()) {
        const aiNode* topNode = nodeStack.top();
        nodeStack.pop();
        glm::mat4 topTransform = transformStack.top();
        transformStack.pop();
        
        /*cout << "  Node " << topNode->mName.C_Str() << " has " << topNode->mNumMeshes << " meshes and " << topNode->mNumChildren << " children.\n";
        if (castMat4(topNode->mTransformation) == glm::mat4(1.0f)) {
            cout << "  Transform: IdentityMtx\n";
        } else {
            cout << "  Transform: " << glm::to_string(castMat4(topNode->mTransformation)) << "\n";
        }*/
        
        auto findResult = nodeSubstitutes->find(string(topNode->mName.C_Str()));
        if (findResult != nodeSubstitutes->end()) {
            auto insertResult = mappedNodes.insert({findResult->second, {topNode, topTransform}});
            if (!insertResult.second) {
                cout << "Warn: Animation contains a duplicate channel name.\n";
            }
        }
        
        for (unsigned int i = 0; i < topNode->mNumChildren; ++i) {
            nodeStack.push(topNode->mChildren[i]);
            transformStack.push(topTransform * castMat4(topNode->mChildren[i]->mTransformation));
        }
    }
    
    cout << "//////////////////////////////////////////////////////////////////////////\n";
    
    stack<const ModelRigged::Node*> refNodeStack;
    refNodeStack.push(referenceModel->getRootNode());
    stack<glm::mat4> refTransformStack;
    refTransformStack.push(referenceModel->getRootNode()->transform);
    while (!refNodeStack.empty()) {
        const ModelRigged::Node* topNode = refNodeStack.top();
        refNodeStack.pop();
        glm::mat4 topTransform = refTransformStack.top();
        refTransformStack.pop();
        
        /*cout << "  Node " << topNode->name << " has " << topNode->children.size() << " children.\n";
        if (topNode->transform == glm::mat4(1.0f)) {
            cout << "  Transform: IdentityMtx\n";
        } else {
            cout << "  Transform: " << glm::to_string(topNode->transform) << "\n";
        }*/
        
        auto findResult = mappedNodes.find(topNode->name);
        if (findResult != mappedNodes.end()) {
            cout << "[" << topNode->name << "]:\n";
            if (topNode->name == "Hips") {
                cout << "Relative:\n";
                cout << glm::to_string(topNode->transform) << "\n";
                cout << glm::to_string(castMat4(findResult->second.first->mTransformation)) << "\n";
                cout << "Combined:\n";
                cout << glm::to_string(topTransform) << "\n";
                cout << glm::to_string(findResult->second.second) << "\n";
            }
            auto findChannel = channels_.find(topNode->name);
            if (findChannel != channels_.end()) {
                float scaling = glm::length(glm::vec3(topNode->transform * glm::vec4(0.0, 0.0, 0.0, 1.0))) / glm::length(findChannel->second.translationKeys[0].first);
                if (topNode->name == "Armature") {
                    scaling = 0.01f;
                }
                cout << "Scaling set to " << scaling << "\n";
                if (topNode->name != "Armature" && topNode->name != "Hips") {
                    for (pair<glm::vec3, double>& key : findChannel->second.translationKeys) {
                        key.first = glm::vec3(topNode->transform * glm::inverse(castMat4(findResult->second.first->mTransformation)) * glm::vec4(key.first, 1.0f));
                    }
                    if (topNode->name == "Left knee" || topNode->name == "Right knee") {
                        glm::quat q = glm::quat_cast(topNode->transform);// * glm::inverse(glm::quat_cast(castMat4(findResult->second.first->mTransformation)));
                        cout << "Quat q = " << glm::to_string(q) << "\nModel node: " << glm::to_string(topNode->transform) << "\nArmature node: " << glm::to_string(castMat4(findResult->second.first->mTransformation)) << "\n";
                        for (pair<glm::quat, double>& key : findChannel->second.rotationKeys) {
                            //key.first = glm::quat_cast(topNode->transform * glm::inverse(castMat4(findResult->second.first->mTransformation)) * mat4_cast(key.first));
                            //key.first = glm::quat_cast(topNode->transform);
                            key.first = q * key.first;
                        }
                    }
                } else {
                    for (pair<glm::vec3, double>& key : findChannel->second.translationKeys) {
                        //key.first = glm::vec3(glm::mat4({0.009, 0.0, 0.0, 0.0}, {0.0, 0.009, 0.0, 0.0}, {0.0, 0.0, 0.009, 0.0}, {0.0, 0.0, 0.0, 1.0}) * glm::vec4(key.first, 1.0));
                        //key.first = glm::vec3(topNode->transform * glm::inverse(castMat4(findResult->second.first->mTransformation)) * glm::vec4(key.first, 1.0f));
                        key.first *= scaling;
                    }
                }
                if (topNode->name == "Left elbow") {
                    cout << "Base elbow length = " << glm::length(glm::vec3(topNode->transform * glm::vec4(0.0, 0.0, 0.0, 1.0))) << "\n";
                    cout << "Converted elbow length = " << glm::length(findChannel->second.translationKeys[0].first) << "\n";
                }
                for (pair<glm::vec3, double>& key : findChannel->second.scalingKeys) {
                    key.first = glm::vec3(1.0f);
                }
            }
        }
        
        for (const ModelRigged::Node* n : topNode->children) {
            refNodeStack.push(n);
            refTransformStack.push(topTransform * n->transform);
        }
    }
}
