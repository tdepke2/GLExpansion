#include "Font.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <algorithm>
#include <iostream>
#include <stdexcept>

Font::Font() {
    _bitmapHandle = 0;
}

Font::~Font() {
    glDeleteTextures(1, &_bitmapHandle);
}

unsigned int Font::getBitmapHandle() const {
    return _bitmapHandle;
}

const glm::uvec2& Font::getBitmapSize() const {
    return _bitmapSize;
}

const unordered_map<char, Font::Glyph>& Font::getGlyphs() const {
    return _glyphs;
}

const glm::uvec2& Font::getGlyphSizeMax() const {
    return _glyphSizeMax;
}

float Font::getLineSpacing() const {
    return _lineSpacing;
}

void Font::loadFont(const string& filename, unsigned int fontSize) {
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        throw runtime_error("Failed to initialize FreeType library.");
    }
    FT_Face face;
    if (FT_New_Face(ft, filename.c_str(), 0, &face)) {
        throw runtime_error("Failed to load font \"" + filename + "\".");
    }
    FT_Set_Pixel_Sizes(face, 0, fontSize);
    
    _lineSpacing = static_cast<float>(face->height) / face->units_per_EM * fontSize;
    _glyphSizeMax = glm::uvec2(0, 0);
    _glyphs.clear();
    for (unsigned char c = 0; c < 128; ++c) {    // Load glyph data and get maximum size.
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            cout << "Error: Failed to load font glyph \"" << c << "\" (" << static_cast<unsigned int>(c) << ").\n";
            continue;
        }
        
        Glyph glyph = {
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x)
        };
        _glyphSizeMax.x = max(_glyphSizeMax.x, face->glyph->bitmap.width);
        _glyphSizeMax.y = max(_glyphSizeMax.y, face->glyph->bitmap.rows);
        
        _glyphs.emplace(c, glyph);
    }
    
    _glyphSizeMax += glm::uvec2(4, 4);    // Add a little extra space to help prevent artifacts.
    
    _bitmapSize = glm::uvec2(_glyphSizeMax.x * 16, _glyphSizeMax.y * 8);
    unsigned char* bitmapData = new unsigned char[_bitmapSize.x * _bitmapSize.y];
    for (unsigned int i = 0; i < _bitmapSize.x * _bitmapSize.y; ++i) {
        bitmapData[i] = 0;
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
    
    if (_bitmapHandle == 0) {    // Create texture for font bitmap.
        glGenTextures(1, &_bitmapHandle);
    }
    glBindTexture(GL_TEXTURE_2D, _bitmapHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, _bitmapSize.x, _bitmapSize.y, 0, GL_RED, GL_UNSIGNED_BYTE, bitmapData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    delete[] bitmapData;
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}
