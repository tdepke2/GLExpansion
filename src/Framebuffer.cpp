#include "Framebuffer.h"
#include <stdexcept>
#include <string>

Framebuffer::Framebuffer(const glm::ivec2& bufferSize) {
    bufferSize_ = bufferSize;
    glGenFramebuffers(1, &framebufferHandle_);
}

Framebuffer::~Framebuffer() {
    glDeleteFramebuffers(1, &framebufferHandle_);
    for (const TextureData& texture : textures_) {
        glDeleteTextures(1, &texture.handle);
    }
    for (const RenderbufferData& renderbuffer : renderbuffers_) {
        glDeleteRenderbuffers(1, &renderbuffer.handle);
    }
}

const glm::ivec2& Framebuffer::getBufferSize() const {
    return bufferSize_;
}

void Framebuffer::setBufferSize(const glm::ivec2& bufferSize) {
    bufferSize_ = bufferSize;
    for (const TextureData& texture : textures_) {
        glBindTexture(GL_TEXTURE_2D, texture.handle);
        glTexImage2D(GL_TEXTURE_2D, 0, texture.internalFormat, bufferSize.x, bufferSize.y, 0, texture.format, texture.type, nullptr);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    
    for (const RenderbufferData& renderbuffer : renderbuffers_) {
        glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer.handle);
        glRenderbufferStorage(GL_RENDERBUFFER, renderbuffer.internalFormat, bufferSize.x, bufferSize.y);
    }
}

void Framebuffer::setDrawBuffers(const vector<GLenum>& attachments) const {
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferHandle_);
    glDrawBuffers(static_cast<int>(attachments.size()), attachments.data());
}

void Framebuffer::attachTexture(GLenum attachment, GLint internalFormat, GLenum format, GLenum type, GLint filter, GLint wrap, const glm::vec4& borderColor) {
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferHandle_);
    
    textures_.emplace_back(0, internalFormat, format, type);
    glGenTextures(1, &textures_.back().handle);    // Create color buffer (using a 2D texture).
    glBindTexture(GL_TEXTURE_2D, textures_.back().handle);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, bufferSize_.x, bufferSize_.y, 0, format, type, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
    if (wrap == GL_CLAMP_TO_BORDER) {
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, value_ptr(borderColor));
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, textures_.back().handle, 0);
}

void Framebuffer::attachRenderbuffer(GLenum attachment, GLenum internalFormat) {
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferHandle_);
    
    renderbuffers_.emplace_back(0, internalFormat);
    glGenRenderbuffers(1, &renderbuffers_.back().handle);    // Create depth and stencil buffer (using a renderbuffer because we don't need to sample values from it like with the color buffer).
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffers_.back().handle);
    glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, bufferSize_.x, bufferSize_.y);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, renderbuffers_.back().handle);
}

void Framebuffer::validate() const {
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferHandle_);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw runtime_error("Framebuffer is not complete. Error code " + to_string(glCheckFramebufferStatus(GL_FRAMEBUFFER)) + ".\n");
    }
}

void Framebuffer::bind(GLenum target) const {
    glBindFramebuffer(target, framebufferHandle_);
}

void Framebuffer::bindTexture(unsigned int index) const {
    glBindTexture(GL_TEXTURE_2D, textures_[index].handle);
}
