#pragma once
#include "engineApi.hpp"
#include <memory>
#include <string>
class Scene;
class ComponentRegistry;
class EditorSelection;
class EditorPlayState;
class LayerRegistry;
class CameraSystem;
class ScriptModule;
struct ComponentCreateContext;
/// Editor document, asset browser, build output and persistence UI.
class EditorProject
{
public:
    ENGINE_API EditorProject(Scene &, ComponentRegistry &, const ComponentCreateContext &, EditorSelection &,
                             EditorPlayState &, LayerRegistry &, CameraSystem &, ScriptModule &);
    ENGINE_API ~EditorProject();
    ENGINE_API void drawFileMenu();
    ENGINE_API void drawPanels();
    ENGINE_API void tick();
    ENGINE_API void buildScripts();
    ENGINE_API bool isBuilding() const;
    /// Component viewer actions for individual user-script source files.
    ENGINE_API bool createScriptComponent(const std::string &name, std::string &error);
    ENGINE_API bool editScriptComponent(const std::string &typeName, std::string &error);
    /// Call after the editor regains focus so saved user source files rebuild.
    ENGINE_API void notifyWindowFocusGained();
    ENGINE_API std::string capture();
    ENGINE_API bool restore(const std::string &document);
private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};
