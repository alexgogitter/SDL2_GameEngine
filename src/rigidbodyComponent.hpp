#pragma once

#include <box2d/box2d.h>
#include <glm/glm.hpp>

#include "component.hpp"
#include "physicsWorld2D.hpp"

enum class BodyType2D
{
    Static,
    Kinematic,
    Dynamic
};

class RigidbodyComponent final : public Component
{
public:
    RigidbodyComponent(
        Object* parent,
        PhysicsWorld2D& physicsWorld,
        BodyType2D bodyType,
        bool fixedRotation = false,
        float gravityScale = 1.0f,
        bool isBullet = false
    );

    ~RigidbodyComponent() override;

    b2BodyId getBodyId() const { return bodyId; }

    void setLinearVelocityPixels(const glm::vec2& pixelsPerSecond);
    glm::vec2 getLinearVelocityPixels() const;

    void applyImpulsePixels(const glm::vec2& impulse);
    void teleportPixels(const glm::vec2& pixelPosition, float angleRadians = 0.0f);

private:
    void Update(std::uint64_t deltaTime) override;

    static b2BodyType toBox2DBodyType(BodyType2D bodyType);

    PhysicsWorld2D* physicsWorld = nullptr;
    b2BodyId bodyId = b2_nullBodyId;
};
