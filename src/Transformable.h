#ifndef TRANSFORMABLE_H_
#define TRANSFORMABLE_H_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

class Transformable {
    public:
    Transformable();
    virtual ~Transformable();
    const glm::vec3& getOrigin() const;
    void setOrigin(const glm::vec3& origin);
    const glm::vec3& getPosition() const;
    void setPosition(const glm::vec3& position);
    const glm::vec3& getPitchYawRoll() const;
    void setPitchYawRoll(const glm::vec3& pitchYawRoll);
    const glm::vec3& getScale() const;
    void setScale(const glm::vec3& scale);
    const glm::mat4& getTransform() const;
    void move(const glm::vec3& distance);
    void rotate(const glm::vec3& pitchYawRoll);
    void scale(const glm::vec3& factor);
    
    private:
    glm::vec3 origin_;
    glm::vec3 position_;
    glm::vec3 pitchYawRoll_;
    glm::vec3 scale_;
    mutable glm::mat4 transform_;
    mutable bool transformChanged_;
};

#endif
