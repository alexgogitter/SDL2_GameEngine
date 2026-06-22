#pragma once

#include <box2d/box2d.h>
#include <glm/glm.hpp>

#include "component.hpp"

class RigidbodyComponent;

class ColliderComponent final : public Component
{
public:
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

    b2ShapeId getShapeId() const { return shapeId; }
    glm::vec2 getSizePixels() const { return sizePixels; }

    // Use this for deliberate local collider deformation. Do not call while Box2D
    // is stepping. For concave destruction, split the result into convex shapes.
    void setBoxSizePixels(const glm::vec2& newSizePixels);

    // Useful for destructible chunks. The parent body remains alive, but this
    // particular collision shape no longer exists.
    void destroyShape();

private:
    void Update(std::uint64_t deltaTime) override;

    RigidbodyComponent* rigidbody = nullptr;
    b2ShapeId shapeId = b2_nullShapeId;
    glm::vec2 sizePixels = {0.0f, 0.0f};
};
