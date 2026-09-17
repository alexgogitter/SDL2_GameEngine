#pragma once

#include <memory>

#include "engineApi.hpp"

class PhysXContext;
class Rigidbody3DComponent;
class PhysicsWorld3D;
/// Owns the active three-dimensional PhysX simulation scene.
class PhysicsWorld3D final
{
  public:
    static constexpr float FixedTimeStep = 1.0f / 60.0f;

    ENGINE_API explicit PhysicsWorld3D(PhysXContext &context);

    ENGINE_API ~PhysicsWorld3D();

    PhysicsWorld3D(const PhysicsWorld3D &) = delete;

    PhysicsWorld3D &operator=(const PhysicsWorld3D &) = delete;

    PhysicsWorld3D(PhysicsWorld3D &&) = delete;

    PhysicsWorld3D &operator=(PhysicsWorld3D &&) = delete;

    ENGINE_API bool isValid() const;

    ENGINE_API void Step(float frameDeltaSeconds);

    ENGINE_API float getLastSimulatedSeconds() const;

    ENGINE_API void *getPhysicsHandle() const;
    ENGINE_API void *getSceneHandle() const;

  private:
    friend class Rigidbody3DComponent;
    friend class BoxCollider3DComponent;

    struct Impl;

    std::unique_ptr<Impl> impl;
    float accumulator = 0.0f;
    float lastSimulatedSeconds = 0.0f;
};
