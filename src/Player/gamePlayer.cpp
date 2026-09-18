#include "window.hpp"
#include "render2D.hpp"
#include "render3D.hpp"
#include "resource_manager.hpp"
#include "scene.hpp"
#include "sceneSerialization.hpp"
#include "componentRegistry.hpp"
#include "builtInComponents.hpp"
#include "scriptModule.hpp"
#include "logger.hpp"
#include "physxContext.hpp"
#include "physicsWorld2D.hpp"
#include "physicsWorld3D.hpp"
#include "cameraSystem.hpp"
#include "cameraComponent.hpp"
#include "eventListener.hpp"
#include <SDL_image.h>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <algorithm>
#include <cstring>

int main(int argc, char **argv)
{
    char *base = SDL_GetBasePath();
    if (!base) return 1;
    std::filesystem::current_path(std::filesystem::u8path(base)); 
    SDL_free(base);
    char *prefs = SDL_GetPrefPath("Ricochet", "GamePlayer");
    Logger::initialize(prefs ? std::string(prefs) + "player.log" : "player.log"); SDL_free(prefs);
    Logger::write(LogLevel::Info, "Player", "Starting game");
    const bool smoke = argc > 1 && std::strcmp(argv[1], "--smoke-test") == 0;
    Window window(1280,720,60,IMG_INIT_PNG,"Ricochet Game");
    if (!window.Initialize()) return 2;
    Renderer2D renderer(window);
    Renderer3D renderer3D(window);
    
    if (renderer.Renderer_Init() != 0 || !renderer3D.Initialize()) return 3;
    int result = 0;
    {
        PhysXContext physics;
        PhysicsWorld3D world3D(physics);
        PhysicsWorld2D world2D(physics);
        if (!physics.isValid() || !world3D.isValid() || !world2D.isValid()) return 4;
        Resource_manager resources;
        ScriptModule scripts;
        ComponentRegistry registry;
        RegisterBuiltInComponents(registry);
        Scene scene(resources,renderer,renderer3D);
        ComponentCreateContext context{&resources,&world2D,&world3D,&renderer,&renderer3D};
        CameraSystem cameras;
        try {
            if (!scripts.reload("scripts/UserScripts.dll", scene, registry, context, true)) throw std::runtime_error("Cannot load game scripts");
            auto config = nlohmann::json::parse(SceneSerialization::readFile("game.json"));
            std::string error;
            auto text = SceneSerialization::readFile(config.at("startupScene").get<std::string>());
            if (!SceneSerialization::restore(scene,text,registry,context,error)) throw std::runtime_error(error);
            auto document = nlohmann::json::parse(text);
            if (document.contains("cameras")) cameras.setPrimaryCamera(scene, CameraOutputTarget::GameView, document.at("cameras").at(0).get<ObjectId>());
            auto &input = EventListener::Get();
            bool quit = false; int frames = 0;
            auto previous = SDL_GetTicks64();
            while (!quit) {
                const auto now = SDL_GetTicks64();
                const auto delta = std::min<Uint64>(now - previous, 50); previous = now;
                input.BeginFrame(); SDL_Event event;
                while (SDL_PollEvent(&event)) { input.ProcessEvent(event); if (event.type == SDL_QUIT) quit = true; }
                if (input.WasKeyPressed(SDL_SCANCODE_ESCAPE)) quit = true;
                world3D.Step(static_cast<float>(delta) / 1000.0f);
                world2D.Step(static_cast<float>(delta) / 1000.0f);
                scene.update(delta);
                auto *camera = cameras.resolvePrimaryCamera(scene, CameraOutputTarget::GameView);
                renderer.SetActiveCamera(camera); renderer3D.SetActiveCamera(camera);
                int width = 0, height = 0; SDL_GL_GetDrawableSize(window.GetSDLWindow(), &width, &height);
                renderer.SetViewportSize(static_cast<float>(width), static_cast<float>(height));
                renderer3D.SetViewportSize(static_cast<float>(width), static_cast<float>(height));
                window.BeginFrame(camera ? camera->getClearColour() : glm::vec4(0.03f,0.04f,0.05f,1));
                renderer.BeginFrame(); renderer3D.BeginFrame();
                scene.draw3D(); scene.draw2D(); renderer3D.Render3D(); renderer.Render2D(); window.Present();
                if (smoke && ++frames >= 5) quit = true;
            }
            renderer.SetActiveCamera(nullptr); renderer3D.SetActiveCamera(nullptr);
        } catch (const std::exception &e) {
            Logger::write(LogLevel::Error,"Player",e.what());
            if (!smoke) SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Game failed to load",e.what(),window.GetSDLWindow());
            result = 5;
        }
    }
    renderer3D.Shutdown(); renderer.Renderer_Close(); window.Shutdown();
    Logger::write(LogLevel::Info,"Player","Game stopped");
    return result;
}
