#ifndef _FRAMEBUFFER_H
#define _FRAMEBUFFER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

using namespace std;

class Framebuffer {
    public:
    Framebuffer(const glm::ivec2& bufferSize);
    ~Framebuffer();
    const glm::ivec2& getBufferSize() const;
    void setBufferSize(const glm::ivec2& bufferSize);
    void attachTexture(GLenum attachment, GLint internalFormat, GLenum format, GLenum type, GLint filter, GLint wrap, const glm::vec4& borderColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    void attachRenderbuffer(GLenum attachment, GLenum internalFormat);
    void disableColorBuffer() const;
    void validate() const;
    void bind() const;
    void bindTexture(unsigned int index) const;
    
    private:
    struct TextureData {
        unsigned int handle;
        GLint internalFormat;
        GLenum format;
        GLenum type;
        
        TextureData(unsigned int handle, GLint internalFormat, GLenum format, GLenum type) : handle(handle), internalFormat(internalFormat), format(format), type(type) {}
    };
    struct RenderbufferData {
        unsigned int handle;
        GLenum internalFormat;
        
        RenderbufferData(unsigned int handle, GLenum internalFormat) : handle(handle), internalFormat(internalFormat) {}
    };
    
    unsigned int _framebufferHandle;
    glm::ivec2 _bufferSize;
    vector<TextureData> _textures;
    vector<RenderbufferData> _renderbuffers;
};

#endif
