#include "Text.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

Text::Text() {
    glGenVertexArrays(1, &_vertexArrayHandle);
    glBindVertexArray(_vertexArrayHandle);
    glGenBuffers(1, &_vertexBufferHandle);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferHandle);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(glm::vec4), nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, false, sizeof(glm::vec4), 0);
}

Text::~Text() {
    glDeleteVertexArrays(1, &_vertexArrayHandle);
    glDeleteBuffers(1, &_vertexBufferHandle);
}

void Text::setFont(shared_ptr<Font> font) {
    _font = font;
}

const string& Text::getString() const {
    return _text;
}

void Text::setString(const string& text) {
    if (_text == text) {
        return;
    }
    _text = text;
    
    vector<glm::vec4> vertices;
    vertices.reserve(6 * _text.length());
    float xPosition = 0.0f, yPosition = 0.0f;
    for (char c : _text) {
        auto findResult = _font->getGlyphs().find(c);
        if (findResult == _font->getGlyphs().end()) {
            cout << "Error: Could not find glyph for character code " << static_cast<unsigned int>(c) << ".\n";
            continue;
        }
        const Font::Glyph* g = &findResult->second;
        
        if (c == ' ') {
            xPosition += (g->advance >> 6);
        } else if (c == '\n') {
            xPosition = 0.0f;
            yPosition -= _font->getLineSpacing();
        } else {
            glm::vec2 pos(xPosition + g->bearing.x, yPosition + g->bearing.y - g->size.y);
            glm::vec2 size(g->size);
            glm::vec2 posTex((static_cast<unsigned int>(c) % 16) * _font->getGlyphSizeMax().x / static_cast<float>(_font->getBitmapSize().x), (static_cast<unsigned int>(c) / 16) * _font->getGlyphSizeMax().y / static_cast<float>(_font->getBitmapSize().y));
            glm::vec2 sizeTex(g->size.x / static_cast<float>(_font->getBitmapSize().x), g->size.y / static_cast<float>(_font->getBitmapSize().y));
            
            vertices.emplace_back(pos.x,          pos.y + size.y, posTex.x,             posTex.y            );
            vertices.emplace_back(pos.x,          pos.y,          posTex.x,             posTex.y + sizeTex.y);
            vertices.emplace_back(pos.x + size.x, pos.y,          posTex.x + sizeTex.x, posTex.y + sizeTex.y);
            
            vertices.emplace_back(pos.x,          pos.y + size.y, posTex.x,             posTex.y            );
            vertices.emplace_back(pos.x + size.x, pos.y,          posTex.x + sizeTex.x, posTex.y + sizeTex.y);
            vertices.emplace_back(pos.x + size.x, pos.y + size.y, posTex.x + sizeTex.x, posTex.y            );
            
            xPosition += (g->advance >> 6);    // Move to next position (advance has units of 1/64 pixels).
        }
    }
    _numVertices = static_cast<unsigned int>(vertices.size());
    
    glBindVertexArray(_vertexArrayHandle);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferHandle);
    int bufferSize;
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);
    if (bufferSize < static_cast<int>(vertices.size() * sizeof(glm::vec4))) {
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec4), vertices.data(), GL_DYNAMIC_DRAW);
    } else {
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(glm::vec4), vertices.data());
    }
}

void Text::draw() const {
    glBindVertexArray(_vertexArrayHandle);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _font->getBitmapHandle());
    glDrawArrays(GL_TRIANGLES, 0, _numVertices);
}
