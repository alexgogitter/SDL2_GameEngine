#pragma once

#include <string>

#include "colliderComponent.hpp"
#include "object.hpp"
#include "physicsWorld2D.hpp"
#include "rigidbodyComponent.hpp"

/// Convenience object composed from a Transform2D, rigid body, and box collider.
class PhysicsBoxObject : public Object
{
public:
    /// Creates a physics rectangle with optional legacy fallback rendering.
    /// @param name Object debug name.
    /// @param resources Resource manager associated with the object lifecycle.
    /// @param renderer Non-owned renderer.
    /// @param physicsWorld Box2D world that owns the body.
    /// @param pixelPosition Initial centre in world pixels.
    /// @param pixelSize Full collider width and height in pixels. Sprite visuals
    ///        should set their own SpriteRendererComponent draw rect.
    /// @param bodyType Static, kinematic, or dynamic Box2D body type.
    /// @param colour Fallback byte-range RGBA colour using 0..255 components.
    /// @param fixedRotation Prevents Box2D angular motion when true.
    /// @param density Collider density.
    /// @param friction Collider friction.
    /// @param restitution Collider bounciness.
    PhysicsBoxObject(
        const std::string& name,
        Resource_manager& resources,
        Renderer2D* renderer,
        PhysicsWorld2D& physicsWorld,
        const glm::vec2& pixelPosition,
        const glm::vec2& pixelSize,
        BodyType2D bodyType,
        const glm::vec4& colour,
        bool fixedRotation = true,
        float density = 1.0f,
        float friction = 0.6f,
        float restitution = 0.0f)
        : Object(name, resources, renderer, nullptr)
    {
        transform.setPosition(pixelPosition);

        // Retained for legacy fallback colour drawing and debug helpers.
        // SpriteRendererComponent uses its own draw rect instead.
        transform.setScale(pixelSize * 0.5f);
        draw_colour = colour;

        rigidbody = new RigidbodyComponent(
            this,
            physicsWorld,
            bodyType,
            fixedRotation
        );

        collider = new ColliderComponent(
            this,
            *rigidbody,
            pixelSize,
            density,
            friction,
            restitution
        );

        add_Component(rigidbody);
        add_Component(collider);
    }

    void Update(std::uint64_t) override
    {
    }

    /// @return Non-owned pointer to the component owned by this object.
    RigidbodyComponent* getRigidbody() const { return rigidbody; }
    /// @return Non-owned pointer to the component owned by this object.
    ColliderComponent* getCollider() const { return collider; }

private:
    RigidbodyComponent* rigidbody = nullptr;
    ColliderComponent* collider = nullptr;
};
