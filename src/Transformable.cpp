#include "Transformable.h"

Transformable::Transformable() : origin_(0.0f), position_(0.0f), pitchYawRoll_(0.0f), scale_(1.0f), transformChanged_(true) {}

Transformable::~Transformable() {}

const glm::vec3& Transformable::getOrigin() const {
    return origin_;
}

void Transformable::setOrigin(const glm::vec3& origin) {
    origin_ = origin;
    transformChanged_ = true;
}

const glm::vec3& Transformable::getPosition() const {
    return position_;
}

void Transformable::setPosition(const glm::vec3& position) {
    position_ = position;
    transformChanged_ = true;
}

const glm::vec3& Transformable::getPitchYawRoll() const {
    return pitchYawRoll_;
}

void Transformable::setPitchYawRoll(const glm::vec3& pitchYawRoll) {
    pitchYawRoll_.x = fmod(pitchYawRoll.x, 2.0f * glm::pi<float>());
    pitchYawRoll_.y = fmod(pitchYawRoll.y, 2.0f * glm::pi<float>());
    pitchYawRoll_.z = fmod(pitchYawRoll.z, 2.0f * glm::pi<float>());
    transformChanged_ = true;
}

const glm::vec3& Transformable::getScale() const {
    return scale_;
}

void Transformable::setScale(const glm::vec3& scale) {
    scale_ = scale;
    transformChanged_ = true;
}

const glm::mat4& Transformable::getTransform() const {
    if (transformChanged_) {
        transform_ = glm::translate(glm::mat4(1.0f), position_);
        transform_ = glm::rotate(transform_, pitchYawRoll_.y, glm::vec3(0.0f, 1.0f, 0.0f));
        transform_ = glm::rotate(transform_, pitchYawRoll_.x, glm::vec3(1.0f, 0.0f, 0.0f));
        transform_ = glm::rotate(transform_, pitchYawRoll_.z, glm::vec3(0.0f, 0.0f, 1.0f));
        transform_ = glm::scale(transform_, scale_);
        transform_ = glm::translate(transform_, -origin_);
        transformChanged_ = false;
        
        /*
        todo: optimize glm::rotate calls in getTransform() #################################################################################
        
        template<typename T, qualifier Q>
        GLM_FUNC_QUALIFIER mat<4, 4, T, Q> rotate(mat<4, 4, T, Q> const& m, T angle, vec<3, T, Q> const& v)
        {
            T const a = angle;
            T const c = cos(a);
            T const s = sin(a);

            vec<3, T, Q> axis(normalize(v));
            vec<3, T, Q> temp((T(1) - c) * axis);

            mat<4, 4, T, Q> Rotate;
            Rotate[0][0] = c + temp[0] * axis[0];
            Rotate[0][1] = temp[0] * axis[1] + s * axis[2];
            Rotate[0][2] = temp[0] * axis[2] - s * axis[1];

            Rotate[1][0] = temp[1] * axis[0] - s * axis[2];
            Rotate[1][1] = c + temp[1] * axis[1];
            Rotate[1][2] = temp[1] * axis[2] + s * axis[0];

            Rotate[2][0] = temp[2] * axis[0] + s * axis[1];
            Rotate[2][1] = temp[2] * axis[1] - s * axis[0];
            Rotate[2][2] = c + temp[2] * axis[2];

            mat<4, 4, T, Q> Result;
            Result[0] = m[0] * Rotate[0][0] + m[1] * Rotate[0][1] + m[2] * Rotate[0][2];
            Result[1] = m[0] * Rotate[1][0] + m[1] * Rotate[1][1] + m[2] * Rotate[1][2];
            Result[2] = m[0] * Rotate[2][0] + m[1] * Rotate[2][1] + m[2] * Rotate[2][2];
            Result[3] = m[3];
            return Result;
        }
        */
    }
    
    return transform_;
}

void Transformable::move(const glm::vec3& distance) {
    setPosition(position_ + distance);
}

void Transformable::rotate(const glm::vec3& pitchYawRoll) {
    setPitchYawRoll(pitchYawRoll_ + pitchYawRoll);
}

void Transformable::scale(const glm::vec3& factor) {
    setScale(scale_ + factor);
}
