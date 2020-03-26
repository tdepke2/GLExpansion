#ifndef _FRAMEBUFFER_H
#define _FRAMEBUFFER_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

class Framebuffer {
    public:
    Framebuffer(const glm::ivec2& bufferSize);
    ~Framebuffer();
    const glm::ivec2& getBufferSize() const;
    void setBufferSize(const glm::ivec2& bufferSize);
    void bind() const;
    void bindTexColorBuffer() const;
    
    private:
    unsigned int _framebufferHandle, _texColorBuffer, _rboDepthStencilBuffer;
    glm::ivec2 _bufferSize;
};

#endif
