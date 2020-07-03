#ifndef _TEXT_RENDER_H
#define _TEXT_RENDER_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <unordered_map>

using namespace std;

class TextRender {
    public:
    unsigned int bitmapHandle;
    
    TextRender();
    ~TextRender();
    const string& getText() const;
    void setText(const string& text);
    void loadFont(const string& filename, unsigned int fontSize);
    void loadFontOldMethod(const string& filename);
    void draw() const;
    void drawOldMethod(const string& s, float x, float y, float scale);
    
    private:
    struct Glyph {
        unsigned int texHandle;
        glm::ivec2 size;    // Size of glyph.
        glm::ivec2 bearing;    // Offset from baseline to top-right of glyph.
        unsigned int advance;    // Offset to advance to next glyph.
    };
    
    string _text;
    glm::uvec2 _bitmapSize;
    glm::uvec2 _glyphSizeMax;
    unordered_map<char, Glyph> glyphs;
    unsigned int textVAO, textVBO;
};

#endif
