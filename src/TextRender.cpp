#include "TextRender.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <algorithm>
#include <iostream>
#include <stdexcept>

TextRender::TextRender() {
    textVAO = 0;
    textVBO = 0;
}

TextRender::~TextRender() {
    glDeleteVertexArrays(1, &textVAO);
    glDeleteBuffers(1, &textVBO);
}

const string& TextRender::getText() const {
    return _text;
}

void TextRender::setText(const string& text) {
    _text = text;
}

void TextRender::loadFont(const string& filename, unsigned int fontSize) {
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        throw runtime_error("Failed to initialize FreeType library.");
    }
    FT_Face face;
    if (FT_New_Face(ft, filename.c_str(), 0, &face)) {
        throw runtime_error("Failed to load font \"" + filename + "\".");
    }
    FT_Set_Pixel_Sizes(face, 0, fontSize);
    
    cout << "Face xMin = " << face->bbox.xMin << ".\n";
    cout << "Face xMax = " << face->bbox.xMax << ".\n";
    cout << "Face yMin = " << face->bbox.yMin << ".\n";
    cout << "Face yMax = " << face->bbox.yMax << ".\n";
    
    _glyphSizeMax = glm::uvec2(0, 0);
    
    for (unsigned char c = 0; c < 128; ++c) {    // Load glyph data and get maximum size.
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            cout << "Error: Failed to load font glyph \"" << c << "\" (" << static_cast<unsigned int>(c) << ").\n";
            continue;
        }
        
        Glyph glyph = {
            0,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x)
        };
        _glyphSizeMax.x = max(_glyphSizeMax.x, face->glyph->bitmap.width);
        _glyphSizeMax.y = max(_glyphSizeMax.y, face->glyph->bitmap.rows);
        
        glyphs.emplace(c, glyph);
    }
    
    _glyphSizeMax += glm::uvec2(2, 2);    // Add a little extra space to help prevent artifacts.
    
    _bitmapSize = glm::uvec2(_glyphSizeMax.x * 16, _glyphSizeMax.y * 8);
    unsigned char* bitmapData = new unsigned char[_bitmapSize.x * _bitmapSize.y];
    for (unsigned int i = 0; i < _bitmapSize.x * _bitmapSize.y; ++i) {
        bitmapData[i] = 127;
    }
    
    for (unsigned char c = 0; c < 128; ++c) {    // Load glyph data again to store into bitmap.
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            continue;
        }
        
        unsigned int offset = (c / 16) * _bitmapSize.x * _glyphSizeMax.y + (c % 16) * _glyphSizeMax.x;
        for (unsigned int i = 0; i < face->glyph->bitmap.width * face->glyph->bitmap.rows; ++i) {
            bitmapData[offset + _bitmapSize.x * (i / face->glyph->bitmap.width) + (i % face->glyph->bitmap.width)] = face->glyph->bitmap.buffer[i];
        }
    }
    
    glGenTextures(1, &bitmapHandle);
    glBindTexture(GL_TEXTURE_2D, bitmapHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, _bitmapSize.x, _bitmapSize.y, 0, GL_RED, GL_UNSIGNED_BYTE, bitmapData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);// GL_LINEAR ##################################################################################################
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    delete[] bitmapData;
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    
    glGenVertexArrays(1, &textVAO);
    glBindVertexArray(textVAO);
    glGenBuffers(1, &textVBO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, 128 * sizeof(glm::vec4), nullptr, GL_DYNAMIC_DRAW);// #########################################################################
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, false, sizeof(glm::vec4), 0);
}

void TextRender::loadFontOldMethod(const string& filename) {
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        throw runtime_error("Failed to initialize FreeType library.");
    }
    
    FT_Face face;
    if (FT_New_Face(ft, filename.c_str(), 0, &face)) {
        throw runtime_error("Failed to load font \"" + filename + "\".");
    }
    
    FT_Set_Pixel_Sizes(face, 0, 48);
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);    // Disable 4-byte alignment restriction when loading glyph textures.
    for (unsigned char c = 0; c < 128; ++c) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            cout << "Error: Failed to load font glyph for character code " << static_cast<unsigned int>(c) << ".\n";
            continue;
        }
        
        unsigned int texHandle;
        glGenTextures(1, &texHandle);
        glBindTexture(GL_TEXTURE_2D, texHandle);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        Glyph glyph = {
            texHandle,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x)
        };
        
        glyphs.emplace(c, glyph);
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    
    glGenVertexArrays(1, &textVAO);
    glBindVertexArray(textVAO);
    glGenBuffers(1, &textVBO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, 6 * 4 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, false, 4 * sizeof(float), 0);
}

void TextRender::draw() const {
    vector<glm::vec4> vertices;
    vertices.reserve(6 * _text.length());
    float xPosition = 0.0f;
    for (char c : _text) {
        auto findResult = glyphs.find(c);
        if (findResult == glyphs.end()) {
            cout << "Error: Could not find glyph for character code " << static_cast<unsigned int>(c) << ".\n";
            continue;
        }
        const Glyph* g = &findResult->second;
        glm::vec2 pos(xPosition + g->bearing.x, g->bearing.y - g->size.y);
        glm::vec2 size(g->size);
        glm::vec2 posTex((static_cast<unsigned int>(c) % 16) * _glyphSizeMax.x / static_cast<float>(_bitmapSize.x), (static_cast<unsigned int>(c) / 16) * _glyphSizeMax.y / static_cast<float>(_bitmapSize.y));
        glm::vec2 sizeTex(g->size.x / static_cast<float>(_bitmapSize.x), g->size.y / static_cast<float>(_bitmapSize.y));
        
        vertices.emplace_back(pos.x,          pos.y + size.y, posTex.x,             posTex.y            );
        vertices.emplace_back(pos.x,          pos.y,          posTex.x,             posTex.y + sizeTex.y);
        vertices.emplace_back(pos.x + size.x, pos.y,          posTex.x + sizeTex.x, posTex.y + sizeTex.y);
        
        vertices.emplace_back(pos.x,          pos.y + size.y, posTex.x,             posTex.y            );
        vertices.emplace_back(pos.x + size.x, pos.y,          posTex.x + sizeTex.x, posTex.y + sizeTex.y);
        vertices.emplace_back(pos.x + size.x, pos.y + size.y, posTex.x + sizeTex.x, posTex.y            );
        
        xPosition += (g->advance >> 6);    // Move to next position (advance has units of 1/64 pixels).
    }
    
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(glm::vec4), vertices.data());
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, bitmapHandle);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(vertices.size()));
}

void TextRender::drawOldMethod(const string& s, float x, float y, float scale) {
    glBindVertexArray(textVAO);
    glActiveTexture(GL_TEXTURE0);
    for (char c : s) {
        Glyph glyph = glyphs[c];
        glm::vec2 pos(x + glyph.bearing.x * scale, y - (glyph.size.y - glyph.bearing.y) * scale);
        glm::vec2 size = glm::vec2(glyph.size) * scale;
        
        float vertices[6][4] = {
            {pos.x,          pos.y + size.y, 0.0f, 0.0f},
            {pos.x,          pos.y,          0.0f, 1.0f},
            {pos.x + size.x, pos.y,          1.0f, 1.0f},
            
            {pos.x,          pos.y + size.y, 0.0f, 0.0f},
            {pos.x + size.x, pos.y,          1.0f, 1.0f},
            {pos.x + size.x, pos.y + size.y, 1.0f, 0.0f}
        };
        
        glBindTexture(GL_TEXTURE_2D, glyph.texHandle);
        
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        x += (glyph.advance >> 6) * scale;    // Move to next position (advance has units of 1/64 pixels).
    }
}
