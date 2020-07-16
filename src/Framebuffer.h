#ifndef FRAMEBUFFER_H_
#define FRAMEBUFFER_H_

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
    void setDrawBuffers(const vector<GLenum>& attachments) const;
    void attachTexture(GLenum attachment, GLint internalFormat, GLenum format, GLenum type, GLint filter, GLint wrap, const glm::vec4& borderColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    void attachRenderbuffer(GLenum attachment, GLenum internalFormat);
    void validate() const;
    void bind(GLenum target = GL_FRAMEBUFFER) const;
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
    
    unsigned int framebufferHandle_;
    glm::ivec2 bufferSize_;
    vector<TextureData> textures_;
    vector<RenderbufferData> renderbuffers_;
};

#endif
