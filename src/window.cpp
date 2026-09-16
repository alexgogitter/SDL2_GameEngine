#include "window.hpp"

#include <cstdio>
#include <iostream>
#include <algorithm>

#include <SDL_image.h>
#include <SDL_ttf.h>
#include <glad/glad.h>

#include "renderTarget.hpp"

Window::Window(
    int width,
    int height,
    int requestedFrameCap,
    int requestedImageFlags,
    const char* requestedTitle)
    : title(requestedTitle),
      screenWidth(width),
      screenHeight(height),
      frameCap(requestedFrameCap),
      imageFlags(requestedImageFlags)
{
}

Window::~Window()
{
    Shutdown();
}

bool Window::Initialize()
{
    if (initialized)
    {
        return true;
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::fprintf(stderr, "Unable to initialise SDL: %s\n", SDL_GetError());
        return false;
    }

    if ((IMG_Init(imageFlags) & imageFlags) != imageFlags)
    {
        std::fprintf(stderr, "Unable to initialise SDL_image: %s\n", IMG_GetError());
        Shutdown();
        return false;
    }

    if (TTF_Init() != 0 || !initializeOpenGLContext())
    {
        Shutdown();
        return false;
    }

    initialized = true;
    return true;
}

bool Window::initializeOpenGLContext()
{
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);

    window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        screenWidth,
        screenHeight,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE |
            SDL_WINDOW_ALLOW_HIGHDPI
    );
    if (window == nullptr)
    {
        std::fprintf(stderr, "Unable to create SDL OpenGL window: %s\n", SDL_GetError());
        return false;
    }

    glContext = SDL_GL_CreateContext(window);
    if (glContext == nullptr)
    {
        std::fprintf(stderr, "Unable to create OpenGL context: %s\n", SDL_GetError());
        return false;
    }

    SDL_GL_MakeCurrent(window, glContext);
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress)))
    {
        std::fprintf(stderr, "Unable to load OpenGL functions through GLAD.\n");
        return false;
    }

    SDL_GL_SetSwapInterval(1);
    std::cout << "OpenGL renderer: " << glGetString(GL_RENDERER) << '\n'
              << "OpenGL version: " << glGetString(GL_VERSION) << '\n';
    return true;
}

void Window::Shutdown()
{
    if (glContext != nullptr)
    {
        SDL_GL_MakeCurrent(window, glContext);
        SDL_GL_DeleteContext(glContext);
        glContext = nullptr;
    }

    if (window != nullptr)
    {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    if (initialized || SDL_WasInit(SDL_INIT_VIDEO) != 0)
    {
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
    }

    fullscreen = false;
    initialized = false;
}

bool Window::SetFullscreen(bool enabled)
{
    if (window == nullptr || SDL_SetWindowFullscreen(
        window,
        enabled ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0) != 0)
    {
        return false;
    }

    fullscreen = enabled;
    return true;
}

bool Window::IsFullscreen() const
{
    return fullscreen;
}

void Window::BeginFrame(const glm::vec4& clearColour)
{
    int drawableWidth = 0;
    int drawableHeight = 0;
    SDL_GL_GetDrawableSize(window, &drawableWidth, &drawableHeight);
    SDL_GetWindowSize(window, &screenWidth, &screenHeight);
    RenderTarget::bindDefault();
    beginFrameOnBoundTarget(drawableWidth, drawableHeight, clearColour);
}

void Window::BeginFrame(RenderTarget& target, const glm::vec4& clearColour)
{
    if (!target.isValid())
    {
        BeginFrame(clearColour);
        return;
    }

    target.bind();
    beginFrameOnBoundTarget(target.getWidth(), target.getHeight(), clearColour);
}

void Window::beginFrameOnBoundTarget(
    int targetWidth,
    int targetHeight,
    const glm::vec4& clearColour)
{
    glViewport(0, 0, std::max(targetWidth, 1), std::max(targetHeight, 1));
    glEnable(GL_FRAMEBUFFER_SRGB);
    glClearColor(clearColour.r, clearColour.g, clearColour.b, clearColour.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void Window::Present()
{
    SDL_GL_SwapWindow(window);
}