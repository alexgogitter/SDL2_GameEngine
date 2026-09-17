#pragma once

#include <memory>

#include <box2d/box2d.h>
#include <glm.hpp>

#include "engineApi.hpp"

class PhysXContext;

/// Owns the fixed-step 2D physics simulation.
///
/// During migration this contains the new PhysX scene and the legacy
/// Box2D world. Box2D will be removed after its components are migrated.
class PhysicsWorld2D
{
  public:
    static constexpr float PixelsPerMetre = 100.0f;
    static constexpr float FixedTimeStep = 1.0f / 60.0f;

    // Temporary Box2D solver setting. Removed after migration.
    static constexpr int SubSteps = 4;

    ENGINE_API explicit PhysicsWorld2D(PhysXContext &physxContext);

    ENGINE_API ~PhysicsWorld2D();

    PhysicsWorld2D(const PhysicsWorld2D &) = delete;
    PhysicsWorld2D &operator=(const PhysicsWorld2D &) = delete;
    PhysicsWorld2D(PhysicsWorld2D &&) = delete;
    PhysicsWorld2D &operator=(PhysicsWorld2D &&) = delete;

    /// Advances PhysX and the temporary legacy world at 60 Hz.
    ENGINE_API void Step(float frameDeltaSeconds);

    /// Returns true when the PhysX scene was created successfully.
    ENGINE_API bool isValid() const;

    // Temporary compatibility used by the Box2D components.
    b2WorldId getWorldId() const { return worldId; }

    static ENGINE_API b2Vec2 pixelsToMetres(const glm::vec2 &pixelPosition);

    static ENGINE_API glm::vec2 metresToPixels(const b2Vec2 &worldPosition);

  private:
    struct PhysXState;

    std::unique_ptr<PhysXState> physxState;

    // Removed after Rigidbody2D and BoxCollider2D move to PhysX.
    b2WorldId worldId = b2_nullWorldId;

    float accumulator = 0.0f;
};