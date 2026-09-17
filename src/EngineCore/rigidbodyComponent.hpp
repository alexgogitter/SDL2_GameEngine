#pragma once

#include <box2d/box2d.h>
#include <glm.hpp>

#include "component.hpp"
#include "physicsWorld2D.hpp"

enum class BodyType2D
{
    Static,
    Kinematic,
    Dynamic
};

/// Box2D body component that synchronizes physics position/rotation to Transform2D.
class RigidbodyComponent final : public Component
{
  public:
    /// Creates a Box2D body at the parent transform's current pixel position.
    /// @param parent Non-owned object synchronized by this component.
    /// @param physicsWorld World that owns the created body.
    /// @param bodyType Static, kinematic, or dynamic behavior.
    /// @param fixedRotation Prevents Box2D angular motion when true.
    /// @param gravityScale Multiplier applied to world gravity.
    /// @param isBullet Enables continuous collision handling for fast bodies.
    RigidbodyComponent(Object *parent, PhysicsWorld2D &physicsWorld, BodyType2D bodyType, bool fixedRotation = false, float gravityScale = 1.0f, bool isBullet = false);

    /// Destroys the Box2D body and all shapes attached to it.
    ~RigidbodyComponent() override;

    /// @return Box2D body identifier owned by this component.
    b2BodyId getBodyId() const { return bodyId; }

    /// Sets linear velocity using engine pixels per second.
    void setLinearVelocityPixels(const glm::vec2 &pixelsPerSecond);
    /// @return Linear velocity in engine pixels per second.
    glm::vec2 getLinearVelocityPixels() const;

    /// Applies a centre impulse converted from engine pixel units to Box2D units.
    void applyImpulsePixels(const glm::vec2 &impulse);
    /// Moves the Box2D body immediately and keeps physics authoritative.
    /// @param pixelPosition New world position in engine pixels.
    /// @param angleRadians New body rotation in radians.
    void teleportPixels(const glm::vec2 &pixelPosition, float angleRadians = 0.0f);

  private:
    void Update(std::uint64_t deltaTime) override;

    static b2BodyType toBox2DBodyType(BodyType2D bodyType);

    PhysicsWorld2D *physicsWorld = nullptr;
    b2BodyId bodyId = b2_nullBodyId;
};
