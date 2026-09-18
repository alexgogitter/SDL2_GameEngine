#pragma once

#include "engineApi.hpp"

#include <cstddef>
#include <memory>

class Component;
class Object;
class PhysicsWorld2D;
class Renderer2D;
class Renderer3D;
class Resource_manager;
class PhysicsWorld3D;

struct ComponentCreateContext
{
    Resource_manager *resources = nullptr;
    PhysicsWorld2D *physicsWorld = nullptr;
    PhysicsWorld3D *physicsWorld3D = nullptr;
    Renderer2D *renderer = nullptr;
    Renderer3D *renderer3D = nullptr;
};

using ComponentFactory = Component *(*)(Object *parent, const ComponentCreateContext &context);

class ComponentRegistry
{
  public:
    ENGINE_API ComponentRegistry();
    ENGINE_API ~ComponentRegistry();
    /// Exchanges registered factories during a validated module reload.
    ENGINE_API void swap(ComponentRegistry &other);

    ComponentRegistry(const ComponentRegistry &) = delete;
    ComponentRegistry &operator=(const ComponentRegistry &) = delete;
    ComponentRegistry(ComponentRegistry &&) = delete;
    ComponentRegistry &operator=(ComponentRegistry &&) = delete;

    ENGINE_API bool registerType(const char *typeName, const char *displayName, const char *category, ComponentFactory factory, const char *const *requiredTypeNames, std::size_t requiredTypeCount);

    ENGINE_API bool unregisterType(const char *typeName);
    ENGINE_API bool contains(const char *typeName) const;

    ENGINE_API std::size_t getTypeCount() const;
    ENGINE_API const char *getTypeNameAt(std::size_t index) const;
    ENGINE_API const char *getDisplayName(const char *typeName) const;

    ENGINE_API const char *getCategory(const char *typeName) const;

    ENGINE_API std::size_t getRequiredTypeCount(const char *typeName) const;

    ENGINE_API const char *getRequiredTypeNameAt(const char *typeName, std::size_t index) const;

    /// Returns the type preventing removal, or an empty string.
    ENGINE_API const char *getRemovalBlocker(const char *typeName, const Object &parent) const;

    /// Removes a component only when nothing depends on it.
    ENGINE_API bool removeFromObject(const char *typeName, Object &parent) const;

    ENGINE_API Component *createAndAttach(const char *typeName, Object &parent, const ComponentCreateContext &context) const;

  private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};
