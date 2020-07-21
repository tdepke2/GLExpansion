#ifndef TEXT_H_
#define TEXT_H_

class Shader;

#include "DrawableInterface.h"
#include "Font.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <string>

using namespace std;

class Text : public DrawableInterface {
    public:
    glm::mat4 modelMtx_;
    
    Text();
    ~Text();
    void setFont(shared_ptr<Font> font);
    const string& getString() const;
    void setString(const string& text);
    void draw(const Shader& shader, const glm::mat4& modelMtx) const;
    
    private:
    shared_ptr<Font> font_;
    unsigned int vertexArrayHandle_, vertexBufferHandle_;
    string text_;
    unsigned int numVertices_;
};

#endif
