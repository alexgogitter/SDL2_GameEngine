#pragma once

#include <box2d/box2d.h>
#include <glm/glm.hpp>

class PhysicsWorld2D
{
public:
    // Box2D should work in metre-like units rather than raw pixels.
    static constexpr float PixelsPerMetre = 100.0f;
    static constexpr float FixedTimeStep = 1.0f / 60.0f;
    static constexpr int SubSteps = 4;

    PhysicsWorld2D();
    ~PhysicsWorld2D();

    PhysicsWorld2D(const PhysicsWorld2D&) = delete;
    PhysicsWorld2D& operator=(const PhysicsWorld2D&) = delete;

    void Step(float frameDeltaSeconds);

    b2WorldId getWorldId() const { return worldId; }

    static b2Vec2 pixelsToMetres(const glm::vec2& pixelPosition);
    static glm::vec2 metresToPixels(const b2Vec2& worldPosition);

private:
    b2WorldId worldId = b2_nullWorldId;
    float accumulator = 0.0f;
};
