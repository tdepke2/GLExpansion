#include "Framebuffer.h"
#include <stdexcept>
#include <string>

Framebuffer::Framebuffer(const glm::ivec2& bufferSize) {
    _bufferSize = bufferSize;
    glGenFramebuffers(1, &_framebufferHandle);
}

Framebuffer::~Framebuffer() {
    glDeleteFramebuffers(1, &_framebufferHandle);
    for (const TextureData& texture : _textures) {
        glDeleteTextures(1, &texture.handle);
    }
    for (const RenderbufferData& renderbuffer : _renderbuffers) {
        glDeleteRenderbuffers(1, &renderbuffer.handle);
    }
}

const glm::ivec2& Framebuffer::getBufferSize() const {
    return _bufferSize;
}

void Framebuffer::setBufferSize(const glm::ivec2& bufferSize) {
    _bufferSize = bufferSize;
    for (const TextureData& texture : _textures) {
        glBindTexture(GL_TEXTURE_2D, texture.handle);
        glTexImage2D(GL_TEXTURE_2D, 0, texture.internalFormat, bufferSize.x, bufferSize.y, 0, texture.format, texture.type, nullptr);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    
    for (const RenderbufferData& renderbuffer : _renderbuffers) {
        glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer.handle);
        glRenderbufferStorage(GL_RENDERBUFFER, renderbuffer.internalFormat, bufferSize.x, bufferSize.y);
    }
}

void Framebuffer::setDrawBuffers(const vector<GLenum>& attachments) const {
    glBindFramebuffer(GL_FRAMEBUFFER, _framebufferHandle);
    glDrawBuffers(static_cast<int>(attachments.size()), attachments.data());
}

void Framebuffer::attachTexture(GLenum attachment, GLint internalFormat, GLenum format, GLenum type, GLint filter, GLint wrap, const glm::vec4& borderColor) {
    glBindFramebuffer(GL_FRAMEBUFFER, _framebufferHandle);
    
    _textures.emplace_back(0, internalFormat, format, type);
    glGenTextures(1, &_textures.back().handle);    // Create color buffer (using a 2D texture).
    glBindTexture(GL_TEXTURE_2D, _textures.back().handle);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, _bufferSize.x, _bufferSize.y, 0, format, type, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
    if (wrap == GL_CLAMP_TO_BORDER) {
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, value_ptr(borderColor));
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, _textures.back().handle, 0);
}

void Framebuffer::attachRenderbuffer(GLenum attachment, GLenum internalFormat) {
    glBindFramebuffer(GL_FRAMEBUFFER, _framebufferHandle);
    
    _renderbuffers.emplace_back(0, internalFormat);
    glGenRenderbuffers(1, &_renderbuffers.back().handle);    // Create depth and stencil buffer (using a renderbuffer because we don't need to sample values from it like with the color buffer).
    glBindRenderbuffer(GL_RENDERBUFFER, _renderbuffers.back().handle);
    glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, _bufferSize.x, _bufferSize.y);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, _renderbuffers.back().handle);
}

void Framebuffer::validate() const {
    glBindFramebuffer(GL_FRAMEBUFFER, _framebufferHandle);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw runtime_error("Framebuffer is not complete. Error code " + to_string(glCheckFramebufferStatus(GL_FRAMEBUFFER)) + ".\n");
    }
}

void Framebuffer::bind(GLenum target) const {
    glBindFramebuffer(target, _framebufferHandle);
}

void Framebuffer::bindTexture(unsigned int index) const {
    glBindTexture(GL_TEXTURE_2D, _textures[index].handle);
}
