#include <SDL.h>
#include <SDL_image.h>
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <imgui.h>

#include "eventListener.hpp"
#include "interfaceImplementation.hpp"
#include "render2D.hpp"
#include "resource_manager.hpp"
#include "time.hpp"

#include "builtInComponents.hpp"
#include "componentInspector.hpp"
#include "componentRegistry.hpp"
#include "editorSelection.hpp"
#include "layerRegistry.hpp"
#include "object.hpp"
#include "objectInspector.hpp"
#include "scene.hpp"
#include "sceneHierarchy.hpp"

#include "physicsWorld2D.hpp"
#include "physicsWorld3D.hpp"
#include "physxContext.hpp"

#include "cameraComponent.hpp"
#include "cameraSystem.hpp"

#include "cameraOutputPanel.hpp"
#include "editorCamera.hpp"
#include "editorProject.hpp"
#include "scriptModule.hpp"
#include "logger.hpp"
#include <filesystem>
#include "editorPlayState.hpp"
#include "editorMenuToolbar.hpp"
#include "editorPlayToolbar.hpp"
#include "gameViewPanel.hpp"
#include "meshFilterComponent.hpp"
#include "render3D.hpp"
#include "renderTarget.hpp"
#include "sceneViewPanel.hpp"
#include "window.hpp"

namespace
{
void SubmitSelectedObjectOutline(Renderer3D &renderer, Resource_manager &resources, Scene &scene, EditorSelection &selection)
{
    Object *selected = selection.getSelectedObject(scene);

    if (selected == nullptr || !selected->isActive()) {
        return;
    }

    MeshFilterComponent *filter = selected->getComponent<MeshFilterComponent>();

    if (filter == nullptr) {
        return;
    }

    MeshRenderState mesh;

    if (!resources.getMeshRenderState(filter->getMesh(), mesh)) {
        return;
    }

    renderer.SubmitSelectionOutline(mesh, selected->getWorldMatrix(), glm::vec4(0.97f, 0.63f, 0.16f, 1.0f), 1.05f, selected->getLayer());
}
} // namespace

int main(int argc, char **argv)
{

    const bool buildTest = argc > 1 && std::strcmp(argv[1], "--build-scripts-test") == 0;
    const bool smokeTest = argc > 1 && std::strcmp(argv[1], "--smoke-test") == 0;
    std::filesystem::current_path(ENGINE_PROJECT_DIRECTORY);
    Logger::initialize("logs/editor.log");
    Logger::write(LogLevel::Info, "Engine", "Starting Ricochet editor");
    Window window(1600, 900, 60, IMG_INIT_PNG, "Ricochet Engine");
    if (!window.Initialize()) {
        return 1;
    }

    Renderer2D renderer(window);
    if (renderer.Renderer_Init() != 0) {
        window.Shutdown();
        return 2;
    }

    Renderer3D renderer3D(window);
    if (!renderer3D.Initialize()) {
        renderer.Renderer_Close();
        window.Shutdown();
        return 3;
    }

    PhysXContext physxContext;

    if (!physxContext.isValid()) {
        renderer3D.Shutdown();
        renderer.Renderer_Close();
        window.Shutdown();
        return 4;
    }

    PhysicsWorld3D physicsWorld(physxContext);
    PhysicsWorld2D physicsWorld2D(physxContext);

    if (!physicsWorld.isValid() || !physicsWorld2D.isValid()) {
        renderer3D.Shutdown();
        renderer.Renderer_Close();
        window.Shutdown();
        return 5;
    }

    Interface *interface = Interface::create(window.GetSDLWindow(), window.GetGLContext());

    if (interface == nullptr) {
        renderer3D.Shutdown();
        renderer.Renderer_Close();
        window.Shutdown();
        return 6;
    }
    else {
        if (smokeTest || buildTest) ImGui::GetIO().IniFilename = nullptr;
        Resource_manager resources;
        ScriptModule scriptModule; // Must outlive the registry and scene.
        ComponentRegistry componentRegistry;

        const bool componentsRegistered = RegisterBuiltInComponents(componentRegistry);

        Scene editorScene(resources, renderer, renderer3D);
        LayerRegistry editorLayers;
        EditorSelection editorSelection;

        CameraSystem cameraSystem;

        RenderTarget gameViewTarget(960, 540);
        GameViewPanelState gameViewPanelState;

        RenderTarget sceneViewTarget(960, 540);
        SceneViewPanelState sceneViewPanelState;

        EditorCamera editorCamera(960.0f, 540.0f);
        EditorPlayState editorPlayState;
        std::string playSnapshot;

        Object *physicsFloorObject = editorScene.createObject("Physics Floor");

        Object *physicsCubeObject = editorScene.createObject("Physics Cube");

        Object *mainCameraObject = editorScene.createObject("Main Camera");

        if (componentsRegistered && mainCameraObject != nullptr) 
        {
            mainCameraObject->draw_colour.a = 0.0f;

            mainCameraObject->transform.setPosition(glm::vec3(0.0f, 4.0f, 10.0f));

            mainCameraObject->transform.setEulerRadians(glm::vec3(glm::radians(-20.0f), 0.0f, 0.0f));

            ComponentCreateContext context;
            context.resources = &resources;
            context.renderer = &renderer;
            context.renderer3D = &renderer3D;
            context.physicsWorld3D = &physicsWorld;

            componentRegistry.createAndAttach("Camera", *mainCameraObject, context);

            editorSelection.selectObject(mainCameraObject);
        }

        if (mainCameraObject != nullptr) 
        {
            cameraSystem.setPrimaryCamera(editorScene, CameraOutputTarget::GameView, mainCameraObject->getId());
        }

        if (componentsRegistered && physicsFloorObject != nullptr) {

            physicsFloorObject->transform.setPosition(glm::vec3(0.0f, -1.0f, 0.0f));

            physicsFloorObject->transform.setScale(glm::vec3(12.0f, 0.5f, 12.0f));

            ComponentCreateContext context;
            context.resources = &resources;
            context.renderer = &renderer;
            context.renderer3D = &renderer3D;
            context.physicsWorld3D = &physicsWorld;

            componentRegistry.createAndAttach("MeshFilter3D", *physicsFloorObject, context);

            componentRegistry.createAndAttach("MeshRenderer3D", *physicsFloorObject, context);

            // No Rigidbody3D is attached. BoxCollider3D
            // therefore creates a static PhysX actor.
            componentRegistry.createAndAttach("BoxCollider3D", *physicsFloorObject, context);
        }

        if (componentsRegistered && physicsCubeObject != nullptr) {
            // MeshFilter does not render by itself. Prevent the
            // object's legacy fallback 2D quad from appearing.
            ComponentCreateContext context;
            context.resources = &resources;
            context.renderer = &renderer;
            context.renderer3D = &renderer3D;
            context.physicsWorld3D = &physicsWorld;

            physicsCubeObject->transform.setPosition(glm::vec3(0.0f, 5.0f, 0.0f));

            componentRegistry.createAndAttach("MeshFilter3D", *physicsCubeObject, context);

            componentRegistry.createAndAttach("MeshRenderer3D", *physicsCubeObject, context);

            componentRegistry.createAndAttach("Rigidbody3D", *physicsCubeObject, context);

            componentRegistry.createAndAttach("BoxCollider3D", *physicsCubeObject, context);

            editorSelection.selectObject(physicsCubeObject);
        }

        ////// =============================== END SCENE POPULATION SETUP ==========================

        ComponentCreateContext editorComponentContext;
        editorComponentContext.resources = &resources;
        editorComponentContext.physicsWorld = &physicsWorld2D;
        editorComponentContext.physicsWorld3D = &physicsWorld;
        editorComponentContext.renderer = &renderer;
        editorComponentContext.renderer3D = &renderer3D;

        scriptModule.reload(ENGINE_SCRIPT_PATH, editorScene, componentRegistry, editorComponentContext, true);
        EditorProject project(editorScene, componentRegistry, editorComponentContext, editorSelection, editorPlayState, editorLayers, cameraSystem, scriptModule);

        if (buildTest) project.buildScripts();

        Time frameTimer;
        std::uint64_t frameDeltaMs = 16;

        interface->addDrawCallback([&editorScene, &editorSelection, &editorLayers, &cameraSystem, &gameViewTarget, &gameViewPanelState, &sceneViewTarget, &sceneViewPanelState, &editorCamera, &editorPlayState, &componentRegistry, &editorComponentContext, &project]() {
            if (gameViewPanelState.fullscreen) {
                DrawGameViewPanel(gameViewTarget, gameViewPanelState);
                return;
            }
            
            DrawMenuToolbar(project);
            project.drawPanels();

            DrawEditorPlayToolbar(editorPlayState);

            ImGui::Begin("Hierarchy");
            DrawSceneHierarchy(editorScene, editorSelection);
            ImGui::End();

            DrawCameraOutputPanel(cameraSystem, editorScene);

            ImGui::Begin("Inspector");

            Object *selected = editorSelection.getSelectedObject(editorScene);

            if (selected != nullptr) {
                ScriptComponentActions scriptActions;
                scriptActions.createScript = [&project](const std::string &name, std::string &error) {
                    return project.createScriptComponent(name, error);
                };
                scriptActions.editScript = [&project](const std::string &typeName, std::string &error) {
                    return project.editScriptComponent(typeName, error);
                };

                DrawObjectInspector(*selected, editorLayers, componentRegistry, editorComponentContext,
                                    editorPlayState.isEditing(), &scriptActions);
            }
            else {
                ImGui::TextDisabled("No object selected");
            }

            ImGui::End();

            DrawGameViewPanel(gameViewTarget, gameViewPanelState);

            DrawSceneViewPanel(sceneViewTarget, editorCamera, editorScene, editorSelection, sceneViewPanelState);
        });

        bool quit = false;
        int smokeFrames = 0;
        SDL_Event event;
        EventListener &input = EventListener::Get();

        while (!quit) {
            project.tick();
            frameTimer.tick();
            input.BeginFrame();

            while (SDL_PollEvent(&event) != 0) {
                input.ProcessEvent(event);
                interface->update(event);

                if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
                    project.notifyWindowFocusGained();
                }

                if (event.type == SDL_QUIT) {
                    quit = true;
                }
            }
            if (input.WasKeyPressed(SDL_SCANCODE_F11)) {
                const bool requestedFullscreen = !gameViewPanelState.fullscreen;

                if (window.SetFullscreen(requestedFullscreen)) {
                    gameViewPanelState.fullscreen = requestedFullscreen;
                }
            }

            if (input.WasKeyPressed(SDL_SCANCODE_ESCAPE)) {
                if (gameViewPanelState.fullscreen) {
                    if (window.SetFullscreen(false)) {
                        gameViewPanelState.fullscreen = false;
                    }
                }
                else {
                    quit = true;
                }
            }

            const float deltaSeconds = static_cast<float>(std::min<std::uint64_t>(frameDeltaMs, 50)) / 1000.0f;

            sceneViewTarget.resize(sceneViewPanelState.requestedWidth, sceneViewPanelState.requestedHeight);

            gameViewTarget.resize(gameViewPanelState.requestedWidth, gameViewPanelState.requestedHeight);

            editorCamera.update(input, sceneViewPanelState.hovered, deltaSeconds);

            if (sceneViewPanelState.hovered && input.WasKeyPressed(SDL_SCANCODE_F)) {
                Object *selected = editorSelection.getSelectedObject(editorScene);

                if (selected != nullptr) {
                    editorCamera.focus(selected->getWorldPosition(), 10.0f);
                }
            }

            const bool advanceSingleFrame = editorPlayState.consumeSingleStepRequest();

            if (editorPlayState.isPlaying() || advanceSingleFrame) {
                const float simulationDelta = advanceSingleFrame ? PhysicsWorld3D::FixedTimeStep : deltaSeconds;

                physicsWorld.Step(simulationDelta);
                physicsWorld2D.Step(simulationDelta);

                const std::uint64_t simulationDeltaMs = advanceSingleFrame ? static_cast<std::uint64_t>(PhysicsWorld3D::FixedTimeStep * 1000.0f) : frameDeltaMs;

                editorScene.update(simulationDeltaMs);
            }

            renderer.SetActiveCamera(&editorCamera.getCamera());
            renderer3D.SetActiveCamera(&editorCamera.getCamera());

            window.BeginFrame(sceneViewTarget, glm::vec4(0.055f, 0.070f, 0.095f, 1.0f));

            renderer.BeginFrame();
            renderer3D.BeginFrame();
            renderer.SetViewportSize(static_cast<float>(sceneViewTarget.getWidth()), static_cast<float>(sceneViewTarget.getHeight()));
            renderer3D.SetViewportSize(static_cast<float>(sceneViewTarget.getWidth()), static_cast<float>(sceneViewTarget.getHeight()));

            editorScene.draw3D();
            editorScene.draw2D();

            SubmitSelectedObjectOutline(renderer3D, resources, editorScene, editorSelection);

            // Opaque 3D geometry first, editor overlays second.
            renderer3D.Render3D();
            renderer.Render2D();

            glm::vec4 gameClearColour(0.012f, 0.018f, 0.030f, 1.0f);

            CameraComponent *gameOutputCamera = cameraSystem.resolvePrimaryCamera(editorScene, CameraOutputTarget::GameView);

            if (gameOutputCamera != nullptr) {
                gameClearColour = gameOutputCamera->getClearColour();
            }

            renderer.SetActiveCamera(gameOutputCamera);
            renderer3D.SetActiveCamera(gameOutputCamera);

            window.BeginFrame(gameViewTarget, gameClearColour);
            renderer.BeginFrame();
            renderer3D.BeginFrame();
            renderer.SetViewportSize(static_cast<float>(gameViewTarget.getWidth()), static_cast<float>(gameViewTarget.getHeight()));
            renderer3D.SetViewportSize(static_cast<float>(gameViewTarget.getWidth()), static_cast<float>(gameViewTarget.getHeight()));

            editorScene.draw3D();
            editorScene.draw2D();

            // The Game View now uses the selected hierarchy camera.
            renderer3D.Render3D();
            renderer.Render2D();

            // The backbuffer contains editor UI, so it must not
            // alter either the Game View or Scene View camera.
            renderer.SetActiveCamera(nullptr);
            renderer3D.SetActiveCamera(nullptr);

            window.BeginFrame({0.035f, 0.040f, 0.050f, 1.0f});

            const EditorPlayMode modeBeforeEditorDraw = editorPlayState.getMode();

            interface->draw();

            const EditorPlayMode modeAfterEditorDraw = editorPlayState.getMode();

            if (modeBeforeEditorDraw == EditorPlayMode::Edit && modeAfterEditorDraw == EditorPlayMode::Playing) {
                playSnapshot = project.capture();
            }
            else if (modeBeforeEditorDraw != EditorPlayMode::Edit && modeAfterEditorDraw == EditorPlayMode::Edit) {
                project.restore(playSnapshot);
                playSnapshot.clear();
            }

            window.Present();
            if (smokeTest && ++smokeFrames >= 5) quit = true;
            if (buildTest && !project.isBuilding()) quit = true;

            frameDeltaMs = frameTimer.tock();
        }
    }

    delete interface;
    renderer.Renderer_Close();
    renderer3D.Shutdown();
    window.Shutdown();
    return 0;
}
