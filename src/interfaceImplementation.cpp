#include "interfaceImplementation.hpp"

Interface* Interface::instance = nullptr;

Interface::Interface(SDL_Window* window, SDL_Renderer* renderer) {
    // Constructor implementation (if needed)
    IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	(void)io;

	ImGui::StyleColorsDark();

	ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
	ImGui_ImplSDLRenderer2_Init(renderer);
}

Interface* Interface::create(SDL_Window* window, SDL_Renderer* renderer) {
    if (!instance) {
        instance = new Interface(window, renderer);
    }
    return instance;
}

Interface* Interface::get() {
    return instance;
}

Interface::~Interface() {
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void Interface::draw(SDL_Renderer* renderer) {
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    for (const auto& callback : drawCallbacks) {
        callback();
    }
    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
}