#include "renderTarget.hpp"

#include <algorithm>
#include <iostream>

#include <glad/glad.h>

RenderTarget::RenderTarget(int initialWidth, int initialHeight) { resize(initialWidth, initialHeight); }

RenderTarget::~RenderTarget() { destroy(); }

bool RenderTarget::resize(int newWidth, int newHeight)
{
    newWidth = std::max(newWidth, 1);
    newHeight = std::max(newHeight, 1);

    if (complete && width == newWidth && height == newHeight) {
        return true;
    }

    GLint previousFramebuffer = 0;

    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFramebuffer);

    if (framebuffer == 0) {
        glGenFramebuffers(1, &framebuffer);
    }

    if (colourTexture == 0) {
        glGenTextures(1, &colourTexture);
    }

    if (depthStencilBuffer == 0) {
        glGenRenderbuffers(1, &depthStencilBuffer);
    }

    width = newWidth;
    height = newHeight;

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glBindTexture(GL_TEXTURE_2D, colourTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colourTexture, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, depthStencilBuffer);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthStencilBuffer);

    complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, static_cast<unsigned int>(previousFramebuffer));

    if (!complete) {
        std::cerr << "Failed to create render target " << width << "x" << height << ".\n";
    }

    return complete;
}

void RenderTarget::bind() const { glBindFramebuffer(GL_FRAMEBUFFER, complete ? framebuffer : 0); }

void RenderTarget::bindDefault() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

bool RenderTarget::isValid() const { return complete; }

int RenderTarget::getWidth() const { return width; }

int RenderTarget::getHeight() const { return height; }

unsigned int RenderTarget::getColourTextureId() const { return colourTexture; }

void RenderTarget::destroy()
{
    if (depthStencilBuffer != 0) {
        glDeleteRenderbuffers(1, &depthStencilBuffer);

        depthStencilBuffer = 0;
    }

    if (colourTexture != 0) {
        glDeleteTextures(1, &colourTexture);

        colourTexture = 0;
    }

    if (framebuffer != 0) {
        glDeleteFramebuffers(1, &framebuffer);

        framebuffer = 0;
    }

    width = 0;
    height = 0;
    complete = false;
}