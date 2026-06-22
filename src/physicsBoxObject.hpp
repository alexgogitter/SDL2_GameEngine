#pragma once

#include <string>

#include "colliderComponent.hpp"
#include "object.hpp"
#include "physicsWorld2D.hpp"
#include "rigidbodyComponent.hpp"

class PhysicsBoxObject : public Object
{
public:
    PhysicsBoxObject(
        const std::string& name,
        Resource_manager& resources,
        Renderer* renderer,
        PhysicsWorld2D& physicsWorld,
        const glm::vec2& pixelPosition,
        const glm::vec2& pixelSize,
        BodyType2D bodyType,
        const glm::vec4& colour,
        bool fixedRotation = true,
        float density = 1.0f,
        float friction = 0.6f,
        float restitution = 0.0f)
        : Object(name, resources, renderer)
    {
        transform.setPosition(pixelPosition);

        // Object::draw currently treats transform scale as half the pixel size.
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

    RigidbodyComponent* getRigidbody() const { return rigidbody; }
    ColliderComponent* getCollider() const { return collider; }

private:
    RigidbodyComponent* rigidbody = nullptr;
    ColliderComponent* collider = nullptr;
};
