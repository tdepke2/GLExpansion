#ifndef DRAWABLE_INTERFACE_H_
#define DRAWABLE_INTERFACE_H_

class Shader;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

class DrawableInterface {
    public:
    virtual ~DrawableInterface();
    virtual void applyInstanceBuffer(unsigned int startIndex) const;
    virtual void draw(const Shader& shader, const glm::mat4& modelMtx) const;
    virtual void drawGeometry() const;
    virtual void drawGeometry(const Shader& shader, const glm::mat4& modelMtx) const;
    virtual void drawInstanced(const Shader& shader, unsigned int count) const;
    virtual void drawGeometryInstanced(const Shader& shader, unsigned int count) const;
};

#endif
