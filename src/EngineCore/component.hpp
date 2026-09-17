#pragma once

#include "engineApi.hpp"
#include <cstdint>
#include <string>

#include "componentProperty.hpp"
#include <cstddef>

class Object;
class Renderer2D;
class Renderer3D;

/// Base class for behavior and renderer-submission units attached to Object.
class Component
{
  private:
    bool created = false;
    bool enabled = true;
    std::string typeName;

  protected:
    Object *parent = nullptr;

    virtual void OnCreate() {}
    virtual void OnEnable() {}
    virtual void OnDisable() {}
    virtual void PreUpdate(std::uint64_t) {}
    virtual void Update(std::uint64_t) {}
    virtual void PostUpdate(std::uint64_t) {}
    virtual void OnDestroy() {}
    virtual void Draw2D(Renderer2D *) {}
    virtual void Draw3D(Renderer3D *) {}

    virtual void OnTransformChanged() {}

    virtual std::size_t GetPropertyCount() const { return 0; }

    virtual bool GetProperty(std::size_t, ComponentProperty &) { return false; }

    virtual void OnPropertyChanged(const char *) {}

  public:
    ENGINE_API Component(const std::string &componentTypeName, Object *parentObject);

    ENGINE_API virtual ~Component();

    Component(const Component &) = delete;
    Component &operator=(const Component &) = delete;
    Component(Component &&) = delete;
    Component &operator=(Component &&) = delete;

    ENGINE_API void create();
    ENGINE_API void preUpdate(std::uint64_t deltaTime);
    ENGINE_API void update(std::uint64_t deltaTime);
    ENGINE_API void postUpdate(std::uint64_t deltaTime);
    ENGINE_API void destroy();
    ENGINE_API void draw2D(Renderer2D *renderer);
    ENGINE_API void draw3D(Renderer3D *renderer);

    ENGINE_API std::size_t getPropertyCount() const;

    ENGINE_API bool getProperty(std::size_t index, ComponentProperty &property);

    ENGINE_API void notifyPropertyChanged(const char *key);

    ENGINE_API void notifyTransformChanged();

    ENGINE_API const std::string &getTypeName() const;

    ENGINE_API void setEnabled(bool value);
    ENGINE_API bool isEnabled() const;
    ENGINE_API bool isCreated() const;

    ENGINE_API Object *getParent() const;
};