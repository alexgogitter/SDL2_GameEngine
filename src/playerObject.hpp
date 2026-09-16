#ifndef PLAYEROBJECT_HPP
#define PLAYEROBJECT_HPP

#include "physicsBoxObject.hpp"

class PlayerObject final : public PhysicsBoxObject
{
public:
    PlayerObject(
        Resource_manager& resources,
        Renderer2D* renderer,
        PhysicsWorld2D& physicsWorld,
        const glm::vec2& pixelPosition)
        : PhysicsBoxObject(
            "Player",
            resources,
            renderer,
            physicsWorld,
            pixelPosition,
            {60.0f, 60.0f},
            BodyType2D::Dynamic,
            {80.0f, 180.0f, 255.0f, 255.0f},
            true,
            1.0f,
            0.7f,
            0.0f)
    {
    }
};

#endif // PLAYEROBJECT_HPP
