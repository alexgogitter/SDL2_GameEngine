#pragma once

#include <memory>

#include "engineApi.hpp"

class PhysicsWorld2D;
class PhysicsWorld3D;
/// Owns the process-wide NVIDIA PhysX SDK state.
class PhysXContext
{
  public:
    ENGINE_API PhysXContext();
    ENGINE_API ~PhysXContext();

    PhysXContext(const PhysXContext &) = delete;
    PhysXContext &operator=(const PhysXContext &) = delete;
    PhysXContext(PhysXContext &&) = delete;
    PhysXContext &operator=(PhysXContext &&) = delete;

    /// Returns true when the PhysX SDK and extensions started successfully.
    ENGINE_API bool isValid() const;
    ENGINE_API void *getPhysicsHandle() const;

  private:
    struct Impl;
    std::unique_ptr<Impl> impl;
    friend class PhysicsWorld2D;
    friend class PhysicsWorld3D;
};