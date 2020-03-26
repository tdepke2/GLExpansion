#include "Framebuffer.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <string>

Framebuffer::Framebuffer(const glm::ivec2& bufferSize) {
    _bufferSize = bufferSize;
    glGenFramebuffers(1, &_framebufferHandle);
    glBindFramebuffer(GL_FRAMEBUFFER, _framebufferHandle);
    
    glGenTextures(1, &_texColorBuffer);    // Create color buffer (using a 2D texture).
    glBindTexture(GL_TEXTURE_2D, _texColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bufferSize.x, bufferSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texColorBuffer, 0);
    
    glGenRenderbuffers(1, &_rboDepthStencilBuffer);    // Create depth and stencil buffer (using a renderbuffer because we don't need to sample values from it like with the color buffer).
    glBindRenderbuffer(GL_RENDERBUFFER, _rboDepthStencilBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, bufferSize.x, bufferSize.y);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rboDepthStencilBuffer);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw runtime_error("Framebuffer is not complete. Error code " + to_string(glCheckFramebufferStatus(GL_FRAMEBUFFER)) + ".\n");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Framebuffer::~Framebuffer() {
    glDeleteFramebuffers(1, &_framebufferHandle);
    glDeleteTextures(1, &_texColorBuffer);
    glDeleteRenderbuffers(1, &_rboDepthStencilBuffer);
}

const glm::ivec2& Framebuffer::getBufferSize() const {
    return _bufferSize;
}

void Framebuffer::setBufferSize(const glm::ivec2& bufferSize) {
    _bufferSize = bufferSize;
    glBindTexture(GL_TEXTURE_2D, _texColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bufferSize.x, bufferSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, _rboDepthStencilBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, bufferSize.x, bufferSize.y);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void Framebuffer::bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, _framebufferHandle);
}

void Framebuffer::bindTexColorBuffer() const {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _texColorBuffer);
}
