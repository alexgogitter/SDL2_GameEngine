#include "physicsWorld2D.hpp"

#include <algorithm>

PhysicsWorld2D::PhysicsWorld2D()
{
    b2WorldDef worldDef = b2DefaultWorldDef();

    // SDL coordinates increase downward, so positive Y produces downward gravity.
    worldDef.gravity = {0.0f, 9.8f};

    worldId = b2CreateWorld(&worldDef);
}

PhysicsWorld2D::~PhysicsWorld2D()
{
    if (b2World_IsValid(worldId))
    {
        b2DestroyWorld(worldId);
    }

    worldId = b2_nullWorldId;
}

void PhysicsWorld2D::Step(float frameDeltaSeconds)
{
    // Prevent a long pause or breakpoint from asking Box2D to simulate hundreds of steps.
    const float clampedDelta = std::clamp(frameDeltaSeconds, 0.0f, 0.25f);
    accumulator += clampedDelta;

    while (accumulator >= FixedTimeStep)
    {
        b2World_Step(worldId, FixedTimeStep, SubSteps);
        accumulator -= FixedTimeStep;
    }
}

b2Vec2 PhysicsWorld2D::pixelsToMetres(const glm::vec2& pixelPosition)
{
    return {
        pixelPosition.x / PixelsPerMetre,
        pixelPosition.y / PixelsPerMetre
    };
}

glm::vec2 PhysicsWorld2D::metresToPixels(const b2Vec2& worldPosition)
{
    return {
        worldPosition.x * PixelsPerMetre,
        worldPosition.y * PixelsPerMetre
    };
}
