#pragma once

#include "engineApi.hpp"

#include <SDL.h>
#include <glm.hpp>

class RenderTarget;

/// Owns the SDL window, OpenGL context, and presentation lifecycle.
class Window
{
  public:
    ENGINE_API Window(int width = 1280, int height = 720, int frameCap = 60, int imageFlags = 0, const char *title = "SDL2 Game Engine");

    ENGINE_API ~Window();

    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;

    ENGINE_API bool Initialize();
    ENGINE_API void Shutdown();

    ENGINE_API bool SetFullscreen(bool enabled);
    ENGINE_API bool IsFullscreen() const;

    ENGINE_API void BeginFrame(const glm::vec4 &clearColour);

    ENGINE_API void BeginFrame(RenderTarget &target, const glm::vec4 &clearColour);

    ENGINE_API void Present();

    SDL_Window *GetSDLWindow() const { return window; }
    SDL_GLContext GetGLContext() const { return glContext; }
    int GetWidth() const { return screenWidth; }
    int GetHeight() const { return screenHeight; }

  private:
    bool initializeOpenGLContext();
    void beginFrameOnBoundTarget(int targetWidth, int targetHeight, const glm::vec4 &clearColour);

    bool initialized = false;
    SDL_Window *window = nullptr;
    SDL_GLContext glContext = nullptr;
    const char *title = "SDL2 Game Engine";
    int screenWidth = 1280;
    int screenHeight = 720;
    int frameCap = 60;
    int imageFlags = 0;
    bool fullscreen = false;
};