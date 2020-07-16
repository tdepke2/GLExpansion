#include "Font.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <algorithm>
#include <iostream>
#include <stdexcept>

Font::Font() {
    bitmapHandle_ = 0;
}

Font::~Font() {
    glDeleteTextures(1, &bitmapHandle_);
}

unsigned int Font::getBitmapHandle() const {
    return bitmapHandle_;
}

const glm::uvec2& Font::getBitmapSize() const {
    return bitmapSize_;
}

const unordered_map<char, Font::Glyph>& Font::getGlyphs() const {
    return glyphs_;
}

const glm::uvec2& Font::getGlyphSizeMax() const {
    return glyphSizeMax_;
}

float Font::getLineSpacing() const {
    return lineSpacing_;
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
    
    lineSpacing_ = static_cast<float>(face->height) / face->units_per_EM * fontSize;
    glyphSizeMax_ = glm::uvec2(0, 0);
    glyphs_.clear();
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
        glyphSizeMax_.x = max(glyphSizeMax_.x, face->glyph->bitmap.width);
        glyphSizeMax_.y = max(glyphSizeMax_.y, face->glyph->bitmap.rows);
        
        glyphs_.emplace(c, glyph);
    }
    
    glyphSizeMax_ += glm::uvec2(4, 4);    // Add a little extra space to help prevent artifacts.
    
    bitmapSize_ = glm::uvec2(glyphSizeMax_.x * 16, glyphSizeMax_.y * 8);
    unsigned char* bitmapData = new unsigned char[bitmapSize_.x * bitmapSize_.y];
    for (unsigned int i = 0; i < bitmapSize_.x * bitmapSize_.y; ++i) {
        bitmapData[i] = 0;
    }
    
    for (unsigned char c = 0; c < 128; ++c) {    // Load glyph data again to store into bitmap.
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            continue;
        }
        
        unsigned int offset = (c / 16) * bitmapSize_.x * glyphSizeMax_.y + (c % 16) * glyphSizeMax_.x;
        for (unsigned int i = 0; i < face->glyph->bitmap.width * face->glyph->bitmap.rows; ++i) {
            bitmapData[offset + bitmapSize_.x * (i / face->glyph->bitmap.width) + (i % face->glyph->bitmap.width)] = face->glyph->bitmap.buffer[i];
        }
    }
    
    if (bitmapHandle_ == 0) {    // Create texture for font bitmap.
        glGenTextures(1, &bitmapHandle_);
    }
    glBindTexture(GL_TEXTURE_2D, bitmapHandle_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, bitmapSize_.x, bitmapSize_.y, 0, GL_RED, GL_UNSIGNED_BYTE, bitmapData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    delete[] bitmapData;
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}
