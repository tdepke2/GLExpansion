#include "Animation.h"
#include <cassert>

Animation::Animation(const string& name, double duration, double ticksPerSecond) : name(name), duration(duration), ticksPerSecond(ticksPerSecond) {}

glm::mat4 Animation::calcChannelTransform(unsigned int channelIndex, double animationTime) const {
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), _interpolateVec3Keys(channels[channelIndex].translationKeys, animationTime));
    transform *= glm::mat4_cast(_interpolateQuatKeys(channels[channelIndex].rotationKeys, animationTime));
    transform = glm::scale(transform, _interpolateVec3Keys(channels[channelIndex].scalingKeys, animationTime));
    
    return transform;
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
    if (i >= keys.size() - 1) {
        assert(false);
    }
    
    float t = static_cast<float>((animationTime - keys[i].second) / (keys[i + 1].second - keys[i].second));
    return glm::mix(keys[i].first, keys[i + 1].first, t);
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
    if (i >= keys.size() - 1) {
        assert(false);
    }
    
    float t = static_cast<float>((animationTime - keys[i].second) / (keys[i + 1].second - keys[i].second));
    return glm::slerp(keys[i].first, keys[i + 1].first, t);
}
