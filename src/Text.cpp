#include "Shader.h"
#include "Text.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

Text::Text() {
    modelMtx_ = glm::mat4(1.0f);
    
    glGenVertexArrays(1, &vertexArrayHandle_);
    glBindVertexArray(vertexArrayHandle_);
    glGenBuffers(1, &vertexBufferHandle_);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferHandle_);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(glm::vec4), nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, false, sizeof(glm::vec4), 0);
}

Text::~Text() {
    glDeleteVertexArrays(1, &vertexArrayHandle_);
    glDeleteBuffers(1, &vertexBufferHandle_);
}

void Text::setFont(shared_ptr<Font> font) {
    font_ = font;
}

const string& Text::getString() const {
    return text_;
}

void Text::setString(const string& text) {
    if (text_ == text) {
        return;
    }
    text_ = text;
    
    vector<glm::vec4> vertices;
    vertices.reserve(6 * text_.length());
    float xPosition = 0.0f, yPosition = 0.0f;
    for (char c : text_) {
        auto findResult = font_->getGlyphs().find(c);
        if (findResult == font_->getGlyphs().end()) {
            cout << "Error: Could not find glyph for character code " << static_cast<unsigned int>(c) << ".\n";
            continue;
        }
        const Font::Glyph* g = &findResult->second;
        
        if (c == ' ') {
            xPosition += (g->advance >> 6);
        } else if (c == '\n') {
            xPosition = 0.0f;
            yPosition -= font_->getLineSpacing();
        } else {
            glm::vec2 pos(xPosition + g->bearing.x, yPosition + g->bearing.y - g->size.y);
            glm::vec2 size(g->size);
            glm::vec2 posTex((static_cast<unsigned int>(c) % 16) * font_->getGlyphSizeMax().x / static_cast<float>(font_->getBitmapSize().x), (static_cast<unsigned int>(c) / 16) * font_->getGlyphSizeMax().y / static_cast<float>(font_->getBitmapSize().y));
            glm::vec2 sizeTex(g->size.x / static_cast<float>(font_->getBitmapSize().x), g->size.y / static_cast<float>(font_->getBitmapSize().y));
            
            vertices.emplace_back(pos.x,          pos.y + size.y, posTex.x,             posTex.y            );
            vertices.emplace_back(pos.x,          pos.y,          posTex.x,             posTex.y + sizeTex.y);
            vertices.emplace_back(pos.x + size.x, pos.y,          posTex.x + sizeTex.x, posTex.y + sizeTex.y);
            
            vertices.emplace_back(pos.x,          pos.y + size.y, posTex.x,             posTex.y            );
            vertices.emplace_back(pos.x + size.x, pos.y,          posTex.x + sizeTex.x, posTex.y + sizeTex.y);
            vertices.emplace_back(pos.x + size.x, pos.y + size.y, posTex.x + sizeTex.x, posTex.y            );
            
            xPosition += (g->advance >> 6);    // Move to next position (advance has units of 1/64 pixels).
        }
    }
    numVertices_ = static_cast<unsigned int>(vertices.size());
    
    glBindVertexArray(vertexArrayHandle_);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferHandle_);
    int bufferSize;
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);
    if (bufferSize < static_cast<int>(vertices.size() * sizeof(glm::vec4))) {
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec4), vertices.data(), GL_DYNAMIC_DRAW);
    } else {
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(glm::vec4), vertices.data());
    }
}

void Text::draw(const Shader& shader, const glm::mat4& modelMtx) const {
    shader.setMat4("modelMtx", modelMtx * modelMtx_);
    glBindVertexArray(vertexArrayHandle_);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, font_->getBitmapHandle());
    glDrawArrays(GL_TRIANGLES, 0, numVertices_);
}
