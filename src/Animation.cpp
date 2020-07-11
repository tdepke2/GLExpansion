#include "Animation.h"
#include <cassert>

Animation::Animation(const string& name, double duration, double ticksPerSecond) : name(name), duration(duration), ticksPerSecond(ticksPerSecond) {}

glm::mat4 Animation::calcChannelTransform(unsigned int channelIndex, double animationTime) const {
    glm::mat4 translationMtx = glm::translate(glm::mat4(1.0f), _interpolateVec3Keys(channels[channelIndex].translationKeys, animationTime));
    glm::mat4 rotationMtx = glm::mat4_cast(_interpolateQuatKeys(channels[channelIndex].rotationKeys, animationTime));
    glm::mat4 scalingMtx = glm::scale(glm::mat4(1.0f), _interpolateVec3Keys(channels[channelIndex].scalingKeys, animationTime));
    
    return translationMtx * rotationMtx * scalingMtx;
}

glm::vec3 Animation::_interpolateVec3Keys(const vector<pair<glm::vec3, double>>& keys, double animationTime) const {
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
    if (i == keys.size() - 1) {
        assert(false);
    }
    
    return keys[i].first;
}

glm::quat Animation::_interpolateQuatKeys(const vector<pair<glm::quat, double>>& keys, double animationTime) const {
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
    if (i == keys.size() - 1) {
        assert(false);
    }
    
    return keys[i].first;
}
