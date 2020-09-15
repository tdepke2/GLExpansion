#include "DampedSpringMotion.h"
#include <cassert>
#include <cmath>

DampedSpringMotion::DampedSpringMotion() : posPosCoef_(1.0f), posVelCoef_(0.0f), velPosCoef_(0.0f), velVelCoef_(1.0f) {}

DampedSpringMotion::DampedSpringMotion(float timeStep, float angularFrequency, float dampingRatio) {
    computeMotionParams(timeStep, angularFrequency, dampingRatio);
}

void DampedSpringMotion::computeMotionParams(float timeStep, float angularFrequency, float dampingRatio) {
    // Adapted from motion equations for a damped spring at http://www.ryanjuckett.com/programming/damped-springs/
    
    constexpr float EPSILON = 0.0001f;
    
    assert(angularFrequency >= 0.0f && dampingRatio >= 0.0f);
    if (angularFrequency < EPSILON) {    // If no angular frequency, spring will not move.
        posPosCoef_ = 1.0f;
        posVelCoef_ = 0.0f;
        velPosCoef_ = 0.0f;
        velVelCoef_ = 1.0f;
        return;
    }
    
    if (dampingRatio > 1.0f + EPSILON) {    // Over-damped spring.
        float za = -angularFrequency * dampingRatio;
        float zb = angularFrequency * sqrt(dampingRatio * dampingRatio - 1.0f);
        float z1 = za - zb;
        float z2 = za + zb;
        
        float e1 = exp(z1 * timeStep);
        float e2 = exp(z2 * timeStep);
        
        float invTwoZb = 1.0f / (2.0f * zb);    // invTwoZb = 1 / (z2 - z1)
        
        float e1OverTwoZb = e1 * invTwoZb;
        float e2OverTwoZb = e2 * invTwoZb;
        
        float z1E1OverTwoZb = z1 * e1OverTwoZb;
        float z2E2OverTwoZb = z2 * e2OverTwoZb;
        
        posPosCoef_ = e1OverTwoZb * z2 - z2E2OverTwoZb + e2;
        posVelCoef_ = -e1OverTwoZb + e2OverTwoZb;
        velPosCoef_ = (z1E1OverTwoZb - z2E2OverTwoZb + e2) * z2;
        velVelCoef_ = -z1E1OverTwoZb + z2E2OverTwoZb;
        
    } else if (dampingRatio < 1.0f - EPSILON) {    // Under-damped spring.
        float omegaZeta = angularFrequency * dampingRatio;
        float alpha = angularFrequency * sqrt(1.0f - dampingRatio * dampingRatio);
        
        float expTerm = exp(-omegaZeta * timeStep);
        float cosTerm = cos(alpha * timeStep);
        float sinTerm = sin(alpha * timeStep);
        
        float invAlpha = 1.0f / alpha;
        
        float expSin = expTerm * sinTerm;
        float expCos = expTerm * cosTerm;
        float expOmegaZetaSinOverAlpha = expTerm * omegaZeta * sinTerm * invAlpha;
        
        posPosCoef_ = expCos + expOmegaZetaSinOverAlpha;
        posVelCoef_ = expSin * invAlpha;
        velPosCoef_ = -expSin * alpha - omegaZeta * expOmegaZetaSinOverAlpha;
        velVelCoef_ = expCos - expOmegaZetaSinOverAlpha;
        
    } else {    // Critically-damped spring.
        float expTerm = exp(-angularFrequency * timeStep);
        float timeExp = timeStep * expTerm;
        float timeExpFreq = timeExp * angularFrequency;
        
        posPosCoef_ = timeExpFreq + expTerm;
        posVelCoef_ = timeExp;
        velPosCoef_ = -angularFrequency * timeExpFreq;
        velVelCoef_ = -timeExpFreq + expTerm;
    }
}

void DampedSpringMotion::updateMotion(float* pos, float* vel, float equilibriumPos) const {
    float oldPos = *pos - equilibriumPos;    // Update in equilibrium relative space.
    float oldVel = *vel;
    
    *pos = oldPos * posPosCoef_ + oldVel * posVelCoef_ + equilibriumPos;
    *vel = oldPos * velPosCoef_ + oldVel * velVelCoef_;
}

void DampedSpringMotion::updateMotion(glm::vec2* pos, glm::vec2* vel, glm::vec2 equilibriumPos) const {
    updateMotion(&(pos->x), &(vel->x), equilibriumPos.x);
    updateMotion(&(pos->y), &(vel->y), equilibriumPos.y);
}

void DampedSpringMotion::updateMotion(glm::vec3* pos, glm::vec3* vel, glm::vec3 equilibriumPos) const {
    updateMotion(&(pos->x), &(vel->x), equilibriumPos.x);
    updateMotion(&(pos->y), &(vel->y), equilibriumPos.y);
    updateMotion(&(pos->z), &(vel->z), equilibriumPos.z);
}
