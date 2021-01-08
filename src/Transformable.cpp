#include "Transformable.h"

Transformable::Transformable() :
    position_(0.0f),
    orientation_(1.0f, 0.0f, 0.0f, 0.0f),
    scale_(1.0f),
    transformChanged_(true) {
}

const glm::vec3& Transformable::getPosition() const {
    return position_;
}

void Transformable::setPosition(const glm::vec3& position) {
    position_ = position;
    transformChanged_ = true;
}

const glm::quat& Transformable::getOrientation() const {
    return orientation_;
}

void Transformable::setOrientation(const glm::quat& orientation) {
    orientation_ = orientation;
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
        transform_ *= glm::mat4_cast(orientation_);
        transform_ = glm::scale(transform_, scale_);
        transformChanged_ = false;
    }
    
    return transform_;
}

void Transformable::move(const glm::vec3& distance) {
    setPosition(position_ + distance);
}

void Transformable::rotate(const glm::quat& rotation) {
    setOrientation(rotation * orientation_);    // confirm this is correct ordering ####################################################
}

void Transformable::scale(const glm::vec3& factor) {
    setScale(scale_ * factor);
}

void Transformable::lookAt(const glm::vec3& point, const glm::vec3& upVec) {
    setOrientation(glm::quat_cast(glm::lookAt(position_, point, upVec)));
    
    // may want to optimize above calculation? #########################################################
    /*glm::lookAt();
    vec<3, T, Q> const f(normalize(center - eye));
    vec<3, T, Q> const s(normalize(cross(f, up)));
    vec<3, T, Q> const u(cross(s, f));

    mat<4, 4, T, Q> Result(1);
    Result[0][0] = s.x;
    Result[1][0] = s.y;
    Result[2][0] = s.z;
    Result[0][1] = u.x;
    Result[1][1] = u.y;
    Result[2][1] = u.z;
    Result[0][2] =-f.x;
    Result[1][2] =-f.y;
    Result[2][2] =-f.z;
    Result[3][0] =-dot(s, eye);
    Result[3][1] =-dot(u, eye);
    Result[3][2] = dot(f, eye);
    return Result;*/
}
