#pragma once

#include "engineApi.hpp"

class RenderTarget final
{
  public:
    ENGINE_API RenderTarget(int initialWidth = 960, int initialHeight = 540);

    ENGINE_API ~RenderTarget();

    RenderTarget(const RenderTarget &) = delete;
    RenderTarget &operator=(const RenderTarget &) = delete;

    ENGINE_API bool resize(int newWidth, int newHeight);

    ENGINE_API void bind() const;
    ENGINE_API static void bindDefault();

    ENGINE_API bool isValid() const;

    ENGINE_API int getWidth() const;
    ENGINE_API int getHeight() const;

    ENGINE_API unsigned int getColourTextureId() const;

  private:
    void destroy();

    unsigned int framebuffer = 0;
    unsigned int colourTexture = 0;
    unsigned int depthStencilBuffer = 0;

    int width = 0;
    int height = 0;
    bool complete = false;
};