#ifndef _TEXT_H
#define _TEXT_H

#include "Font.h"
#include "Shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <string>

using namespace std;

class Text {
    public:
    glm::mat4 modelMtx;
    
    Text();
    ~Text();
    void setFont(shared_ptr<Font> font);
    const string& getString() const;
    void setString(const string& text);
    void draw(const Shader* shader, const glm::mat4& modelMtx = glm::mat4(1.0f)) const;
    
    private:
    shared_ptr<Font> _font;
    unsigned int _vertexArrayHandle, _vertexBufferHandle;
    string _text;
    unsigned int _numVertices;
};

#endif
