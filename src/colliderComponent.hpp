#pragma once

#include <box2d/box2d.h>
#include <glm/glm.hpp>

#include "component.hpp"

class RigidbodyComponent;

/// Box-shaped Box2D collider attached to an existing RigidbodyComponent.
class ColliderComponent final : public Component
{
public:
    /// Creates a Box2D polygon shape using a full pixel width and height.
    /// @param parent Non-owned object used for component identity.
    /// @param rigidbody Body that receives the new shape.
    /// @param sizePixels Full collider width and height in pixels.
    /// @param density Mass density used by dynamic bodies.
    /// @param friction Surface friction coefficient.
    /// @param restitution Surface bounciness coefficient.
    /// @param isSensor Enables overlap events without physical response.
    ColliderComponent(
        Object* parent,
        RigidbodyComponent& rigidbody,
        const glm::vec2& sizePixels,
        float density = 1.0f,
        float friction = 0.6f,
        float restitution = 0.0f,
        bool isSensor = false
    );

    // The rigidbody owns the Box2D body. It automatically destroys attached
    // shapes when the parent object is destroyed.
    ~ColliderComponent() override = default;

    /// @return Box2D shape identifier owned indirectly by the rigid body.
    b2ShapeId getShapeId() const { return shapeId; }
    /// @return Full collider width and height in pixels for physics and debug draw.
    glm::vec2 getSizePixels() const { return sizePixels; }

    // Use this for deliberate local collider deformation. Do not call while Box2D
    // is stepping. For concave destruction, split the result into convex shapes.
    /// Replaces box geometry and refreshes dynamic-body mass.
    /// Do not call while Box2D is stepping.
    void setBoxSizePixels(const glm::vec2& newSizePixels);

    // Useful for destructible chunks. The parent body remains alive, but this
    // particular collision shape no longer exists.
    /// Destroys this collision shape while leaving the parent body alive.
    void destroyShape();

private:
    void Update(std::uint64_t deltaTime) override;

    RigidbodyComponent* rigidbody = nullptr;
    b2ShapeId shapeId = b2_nullShapeId;
    glm::vec2 sizePixels = {0.0f, 0.0f};
};
