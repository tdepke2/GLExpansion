#ifndef DAMPED_SPRING_MOTION_H_
#define DAMPED_SPRING_MOTION_H_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

class DampedSpringMotion {
    public:
    DampedSpringMotion();
    DampedSpringMotion(float timeStep, float angularFrequency, float dampingRatio);
    void computeMotionParams(float timeStep, float angularFrequency, float dampingRatio);
    void updateMotion(float* pos, float* vel, float equilibriumPos) const;
    void updateMotion(glm::vec2* pos, glm::vec2* vel, glm::vec2 equilibriumPos) const;
    void updateMotion(glm::vec3* pos, glm::vec3* vel, glm::vec3 equilibriumPos) const;
    
    private:
    float posPosCoef_, posVelCoef_;
    float velPosCoef_, velVelCoef_;
};

#endif
