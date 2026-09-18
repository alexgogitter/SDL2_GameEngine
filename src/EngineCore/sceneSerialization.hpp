#pragma once
#include "engineApi.hpp"
#include "objectId.hpp"
#include <string>
class Scene;
class Object;
class ComponentRegistry;
struct ComponentCreateContext;

/// Versioned JSON persistence shared by the editor, player and C++ reload system.
namespace SceneSerialization
{
ENGINE_API std::string capture(const Scene &scene);
/// Builds a replacement scene first. Failure leaves the live scene untouched.
ENGINE_API bool restore(Scene &scene, const std::string &json, const ComponentRegistry &registry,
                        const ComponentCreateContext &context, std::string &error);
/// Writes through a temporary sibling file, replacing the destination only after a successful write.
ENGINE_API bool save(const Scene &scene, const std::string &path, std::string &error);
ENGINE_API bool load(Scene &scene, const std::string &path, const ComponentRegistry &registry,
                     const ComponentCreateContext &context, std::string &error);
ENGINE_API bool savePrefab(const Object &object, const std::string &path, std::string &error);
/// Creates an independent copy of a saved hierarchy with fresh object IDs.
ENGINE_API Object *instantiatePrefab(Scene &scene, const std::string &path, const ComponentRegistry &registry,
                                    const ComponentCreateContext &context, std::string &error);
ENGINE_API bool writeFile(const std::string &path, const std::string &contents, std::string &error);
ENGINE_API std::string readFile(const std::string &path);
}
