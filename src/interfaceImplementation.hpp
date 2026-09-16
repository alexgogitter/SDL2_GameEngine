#pragma once
#include "engineApi.hpp"
#include <functional>
#include <memory>
#include <vector>

#include <SDL.h>
#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>

class Shader;

/// Owns the Dear ImGui context, SDL event integration, and OpenGL UI resources.
class Interface
{
public:
    /// Creates the singleton interface on the supplied current OpenGL context.
    /// @return Existing singleton when already created.
    static ENGINE_API Interface* create(SDL_Window* window, SDL_GLContext glContext);
    /// @return Current singleton, or nullptr before create().
    static ENGINE_API Interface* get();

    /// Releases OpenGL UI resources and destroys the ImGui context.
    ENGINE_API ~Interface();

    /// Registers UI code invoked between ImGui::NewFrame() and ImGui::Render().
    ENGINE_API void addDrawCallback(std::function<void()> callback);

    /// Builds and renders the current ImGui frame. Call after Renderer::Render2D().
    ENGINE_API void draw();

    /// Forwards one SDL event to the ImGui SDL2 platform backend.
    ENGINE_API void update(SDL_Event& event);

    Interface(const Interface&) = delete;
    Interface& operator=(const Interface&) = delete;

private:
    Interface(SDL_Window* window, SDL_GLContext glContext);

    bool initialiseOpenGLRenderer();
    void renderDrawData(ImDrawData* drawData);

    static Interface* instance;

    SDL_Window* window = nullptr;
    SDL_GLContext glContext = nullptr;
    std::vector<std::function<void()>> drawCallbacks;

    std::unique_ptr<Shader> shader;
    unsigned int fontTexture = 0;
    unsigned int vertexArray = 0;
    unsigned int vertexBuffer = 0;
    unsigned int indexBuffer = 0;
};
