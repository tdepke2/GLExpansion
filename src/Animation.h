#ifndef ANIMATION_H_
#define ANIMATION_H_

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <string>
#include <unordered_map>
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
    
    unordered_map<string, Channel> channels_;
    string name_;
    double duration_, ticksPerSecond_;
    
    static void loadFile(const string& filename, unordered_map<string, Animation>* animations, const string& repairFilename = "");
    Animation(const string& name, double duration, double ticksPerSecond);
    Animation(const aiScene* scene, unsigned int index, unordered_map<string, string>* nodeSubstitutes = nullptr);
    void loadFromScene(const aiScene* scene, unsigned int index, unordered_map<string, string>* nodeSubstitutes);
    glm::mat4 calcChannelTransform(const Channel& channel, double animationTime) const;
    
    private:
    static inline glm::vec3 castVec3(const aiVector3D& v) { return glm::vec3(v.x, v.y, v.z); }
    static inline glm::quat castQuat(const aiQuaternion& q) { return glm::quat(q.w, q.x, q.y, q.z); }
    glm::vec3 interpolateVec3Keys(const vector<pair<glm::vec3, double>>& keys, double animationTime) const;
    glm::quat interpolateQuatKeys(const vector<pair<glm::quat, double>>& keys, double animationTime) const;
};

#endif
