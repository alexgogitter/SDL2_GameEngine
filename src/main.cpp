#include <cstdint>
#include <cmath>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

#include "eventListener.hpp"
#include "fpsCounter.hpp"
#include "interfaceImplementation.hpp"
#include "physicsBoxObject.hpp"
#include "physicsWorld2D.hpp"
#include "playerObject.hpp"
#include "render.hpp"
#include "resource_manager.hpp"
#include "textureComponent.hpp"
#include "time.hpp"

std::string fontPath = "res/fonts/comicz.ttf";

std::uint64_t frameDeltaMs = 16;
std::uint64_t fpsSampleElapsedMs = 0;
int fpsSampleFrameCount = 0;
float framesPerSecond = 0.0f;
std::size_t spawnedSquareCount = 0;

void demoCallback()
{
    ImGui::Begin("Physics Test Scene");
    ImGui::Text("Left-click: spawn a physics square");
    ImGui::Text("Blue: player | Green: platform | Stone: spawned squares");
    ImGui::Text("Spawned squares: %zu", spawnedSquareCount);
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::End();
}

int main(int, char**)
{
    Renderer renderer(1080, 1920, 60, IMG_INIT_PNG, "Box2D Physics Test");
    if (renderer.Renderer_Init() != 0)
    {
        return 1;
    }

    Interface* interface = Interface::create(
        renderer.get_SDLWindow(),
        renderer.get_SDLRenderer()
    );

    {
        Resource_manager resources(renderer.get_SDLRenderer());
        PhysicsWorld2D physicsWorld;
        fpsCounter fpsCounterObject;
        Time frameTimer;

        // Unique ownership is essential because left-click creates objects at runtime.
        // physicsWorld is declared before this vector, so all bodies are destroyed
        // before the Box2D world is destroyed when this scope ends.
        std::vector<std::unique_ptr<Object>> gameObjects;

        int screenWidth = 0;
        int screenHeight = 0;
        SDL_GetRendererOutputSize(
            renderer.get_SDLRenderer(),
            &screenWidth,
            &screenHeight
        );

        interface->addDrawCallback(demoCallback);

        gameObjects.push_back(std::make_unique<PhysicsBoxObject>(
            "Platform",
            resources,
            &renderer,
            physicsWorld,
            glm::vec2(screenWidth * 0.5f, screenHeight - 130.0f),
            glm::vec2(900.0f, 50.0f),
            BodyType2D::Static,
            glm::vec4(75.0f, 185.0f, 115.0f, 255.0f),
            true,
            1.0f,
            0.9f,
            0.0f
        ));

        gameObjects.push_back(std::make_unique<PlayerObject>(
            resources,
            &renderer,
            physicsWorld,
            glm::vec2(screenWidth * 0.5f, 130.0f)
        ));

        bool quit = false;
        SDL_Event event;
        EventListener& eventListener = EventListener::Get();

        while (!quit)
        {
            frameTimer.tick();
            eventListener.BeginFrame();

            while (SDL_PollEvent(&event) != 0)
            {
                eventListener.ProcessEvent(event);
                interface->update(event);

                if (event.type == SDL_QUIT)
                {
                    quit = true;
                }
            }

            // Mouse events have been processed, so this uses the click position
            // from the current frame rather than a frame-old value.
            if (eventListener.WasMouseButtonPressed(SDL_BUTTON_LEFT) &&
                !ImGui::GetIO().WantCaptureMouse)
            {
                const EventListener::MouseState& mouse = eventListener.GetMouseState();

                auto spawnedSquare = std::make_unique<PhysicsBoxObject>(
                    "SpawnedSquare",
                    resources,
                    &renderer,
                    physicsWorld,
                    glm::vec2(static_cast<float>(mouse.x), static_cast<float>(mouse.y)),
                    glm::vec2(42.0f, 42.0f),
                    BodyType2D::Dynamic,
                    glm::vec4(245.0f, 145.0f, 45.0f, 255.0f),
                    false,
                    3.0f,
                    1.0f,
                    0.15f
                );

                spawnedSquare->add_Component(new TextureComponent(
                    spawnedSquare.get(),
                    resources,
                    "res/textures/2D-Ground/blktex_stone_01.png"
                ));

                gameObjects.push_back(std::move(spawnedSquare));

                ++spawnedSquareCount;
            }

            physicsWorld.Step(static_cast<float>(frameDeltaMs) / 1000.0f);

            SDL_SetRenderDrawColor(renderer.get_SDLRenderer(), 28, 30, 38, 255);
            SDL_RenderClear(renderer.get_SDLRenderer());

            for (const std::unique_ptr<Object>& object : gameObjects)
            {
                object->update(frameDeltaMs);
                object->draw(&renderer);
            }

            interface->draw(renderer.get_SDLRenderer());

            std::ostringstream fpsText;
            fpsText << std::floor(framesPerSecond);
            fpsCounterObject.update(&renderer, fpsText.str());

            SDL_RenderPresent(renderer.get_SDLRenderer());

            frameDeltaMs = frameTimer.tock();
            fpsSampleElapsedMs += frameDeltaMs;
            ++fpsSampleFrameCount;

            if (fpsSampleFrameCount >= 100)
            {
                if (fpsSampleElapsedMs > 0)
                {
                    framesPerSecond =
                        (1000.0f * static_cast<float>(fpsSampleFrameCount)) /
                        static_cast<float>(fpsSampleElapsedMs);
                }

                fpsSampleFrameCount = 0;
                fpsSampleElapsedMs = 0;
            }
        }
    }

    delete interface;
    renderer.Renderer_Close();

    return 0;
}
