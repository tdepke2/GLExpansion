#ifndef FONT_H_
#define FONT_H_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <unordered_map>

using namespace std;

class Font {
    public:
    struct Glyph {
        glm::ivec2 size;    // Size of glyph.
        glm::ivec2 bearing;    // Offset from baseline to top-right of glyph.
        unsigned int advance;    // Offset to advance to next glyph.
    };
    
    Font(const string& filename, unsigned int fontSize);
    ~Font();
    unsigned int getBitmapHandle() const;
    const glm::uvec2& getBitmapSize() const;
    const unordered_map<char, Glyph>& getGlyphs() const;
    const glm::uvec2& getGlyphSizeMax() const;
    float getLineSpacing() const;
    void loadFont(const string& filename, unsigned int fontSize);
    
    private:
    unsigned int bitmapHandle_;
    glm::uvec2 bitmapSize_;
    unordered_map<char, Glyph> glyphs_;
    glm::uvec2 glyphSizeMax_;
    float lineSpacing_;
};

#endif
