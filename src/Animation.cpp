#include "Animation.h"
#include <cassert>
#include <iostream>

void Animation::loadFile(const string& filename, unordered_map<string, Animation>* animations) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filename, aiProcess_JoinIdenticalVertices | aiProcess_Triangulate);
    
    if (!scene || !scene->mRootNode) {
        cout << "Failed to load model file \"" << filename << "\": " << importer.GetErrorString() << "\n";
        return;
    }
    
    for (unsigned int i = 0; i < scene->mNumAnimations; ++i) {    // Load all animations of the model.
        auto insertResult = animations->insert({string(scene->mAnimations[i]->mName.C_Str()), Animation(scene, i)});
        if (!insertResult.second) {
            cout << "Error: Found animation with the same name as an existing one.\n";
        }
    }
}

Animation::Animation(const string& name, double duration, double ticksPerSecond) : name_(name), duration_(duration), ticksPerSecond_(ticksPerSecond) {}

Animation::Animation(const aiScene* scene, unsigned int index) {
    loadFromScene(scene, index);
}

void Animation::loadFromScene(const aiScene* scene, unsigned int index) {
    name_ = string(scene->mAnimations[index]->mName.C_Str());
    duration_ = scene->mAnimations[index]->mDuration;
    ticksPerSecond_ = (scene->mAnimations[index]->mTicksPerSecond != 0.0 ? scene->mAnimations[index]->mTicksPerSecond : 20.0);
    
    //cout << "Loading animation " << name_ << " with duration " << duration_ << "s at " << ticksPerSecond_ << " TPS.\n";
    
    for (unsigned int j = 0; j < scene->mAnimations[index]->mNumChannels; ++j) {
        const aiNodeAnim* nodeAnim = scene->mAnimations[index]->mChannels[j];
        //cout << "  Channel " << nodeAnim->mNodeName.C_Str() << " has " << nodeAnim->mNumPositionKeys << " position, " << nodeAnim->mNumRotationKeys << " rotation, and " << nodeAnim->mNumScalingKeys << " scaling keys.\n";
        
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
        
        auto insertResult = channels_.insert({string(nodeAnim->mNodeName.C_Str()), channel});
        if (!insertResult.second) {
            cout << "Warn: Animation contains a duplicate channel name.\n";
        }
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
