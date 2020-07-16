#ifndef ANIMATION_H_
#define ANIMATION_H_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <utility>
#include <vector>

using namespace std;

class Animation {
    public:
    struct Channel {
        vector<pair<glm::vec3, double>> translationKeys;
        vector<pair<glm::quat, double>> rotationKeys;
        vector<pair<glm::vec3, double>> scalingKeys;
    };
    
    vector<Channel> channels_;
    string name_;
    double duration_, ticksPerSecond_;
    
    Animation(const string& name, double duration, double ticksPerSecond);
    glm::mat4 calcChannelTransform(unsigned int channelIndex, double animationTime) const;
    
    private:
    glm::vec3 interpolateVec3Keys(const vector<pair<glm::vec3, double>>& keys, double animationTime) const;
    glm::quat interpolateQuatKeys(const vector<pair<glm::quat, double>>& keys, double animationTime) const;
};

#endif
