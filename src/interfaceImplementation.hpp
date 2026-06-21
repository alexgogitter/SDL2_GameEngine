#ifndef INTERFACEIMPLEMENTATION_HPP
#define INTERFACEIMPLEMENTATION_HPP

#include <functional>
#include <vector>
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"
// forward declarations to avoid pulling SDL headers here
struct SDL_Window;
struct SDL_Renderer;



class Interface {
public:
    // create the singleton (only call once)
    static Interface* create(SDL_Window* window, SDL_Renderer* renderer);
    // get the existing singleton (returns nullptr if not created)
    static Interface* get();

    ~Interface();
    void addDrawCallback(std::function<void()> callback) {
        drawCallbacks.push_back(callback);
    }
    void draw(SDL_Renderer* renderer);
    void update(SDL_Event& e) {
        ImGui_ImplSDL2_ProcessEvent(&e);
    };
    // prevent copies/moves
    Interface(const Interface&) = delete;
    Interface& operator=(const Interface&) = delete;
    Interface(Interface&&) = delete;
    Interface& operator=(Interface&&) = delete;
    
private:
    // private constructor for singleton
    Interface(SDL_Window* window, SDL_Renderer* renderer);
    static Interface* instance;

    SDL_Window* window;
    SDL_Renderer* renderer;
    std::vector<std::function<void()>> drawCallbacks;

};

#endif // INTERFACEIMPLEMENTATION_HPP