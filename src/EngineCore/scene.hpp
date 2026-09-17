#pragma once

#include "engineApi.hpp"
#include "objectId.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

class Object;
class Renderer2D;
class Renderer3D;
class Resource_manager;

class Scene
{
  public:
    ENGINE_API Scene(Resource_manager &resources, Renderer2D &renderer, Renderer3D &renderer3D);

    ENGINE_API ~Scene();

    Scene(const Scene &) = delete;
    Scene &operator=(const Scene &) = delete;
    Scene(Scene &&) = delete;
    Scene &operator=(Scene &&) = delete;

    ENGINE_API Object *createObject(const std::string &name, ObjectId requestedId = InvalidObjectId);

    ENGINE_API Object *createChildObject(ObjectId parentId, const std::string &name, ObjectId requestedId = InvalidObjectId);

    ENGINE_API bool destroyObject(ObjectId objectId);
    /// Changes an object's parent.
    ///
    /// Pass InvalidObjectId as newParentId to make the object a root.
    /// When keepWorldTransform is true, the visible world transform is preserved.
    ENGINE_API bool reparentObject(ObjectId objectId, ObjectId newParentId, bool keepWorldTransform = true);
    ENGINE_API Object *findObject(ObjectId objectId);

    ENGINE_API std::size_t getRootObjectCount() const;
    ENGINE_API Object *getRootObject(std::size_t index) const;

    ENGINE_API void update(std::uint64_t deltaTime);
    ENGINE_API void draw2D();
    ENGINE_API void draw3D();

  private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};