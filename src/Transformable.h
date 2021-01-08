#ifndef TRANSFORMABLE_H_
#define TRANSFORMABLE_H_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

class Transformable {
    public:
    Transformable();
    const glm::vec3& getPosition() const;
    void setPosition(const glm::vec3& position);
    const glm::quat& getOrientation() const;
    void setOrientation(const glm::quat& orientation);
    const glm::vec3& getScale() const;
    void setScale(const glm::vec3& scale);
    const glm::mat4& getTransform() const;
    void move(const glm::vec3& distance);
    void rotate(const glm::quat& rotation);
    void scale(const glm::vec3& factor);
    void lookAt(const glm::vec3& point, const glm::vec3& upVec);
    
    private:
    glm::vec3 position_;
    glm::quat orientation_;
    glm::vec3 scale_;
    mutable glm::mat4 transform_;
    mutable bool transformChanged_;
};

#endif
