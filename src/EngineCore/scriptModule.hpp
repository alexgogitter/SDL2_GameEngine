#pragma once
#include "engineApi.hpp"
#include <memory>
#include <string>
class Scene;
class ComponentRegistry;
struct ComponentCreateContext;
/// Owns a shadow-loaded gameplay DLL. Must outlive every scene containing its components.
class ScriptModule
{
public:
    ENGINE_API ScriptModule();
    ENGINE_API ~ScriptModule();
    /// Checks changed DLL content at frame boundaries, loading and staging before committing.
    ENGINE_API bool reload(const std::string &path, Scene &scene, ComponentRegistry &registry,
                           const ComponentCreateContext &context, bool force = false);
private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};
