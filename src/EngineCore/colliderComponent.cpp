#include "colliderComponent.hpp"
#include "rigidbodyComponent.hpp"

ColliderComponent::ColliderComponent(Object *parentObject, RigidbodyComponent &rigidbodyComponent, const glm::vec2 &colliderSizePixels, float density, float friction, float restitution, bool isSensor) : Component("BoxCollider2D", parentObject), rigidbody(&rigidbodyComponent), sizePixels(colliderSizePixels)
{
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = density;
    shapeDef.material.friction = friction;
    shapeDef.material.restitution = restitution;
    shapeDef.isSensor = isSensor;

    // Box2D v3 stores events after stepping. Enable only the relevant stream.
    shapeDef.enableContactEvents = !isSensor;
    shapeDef.enableSensorEvents = isSensor;

    // This is deliberately the collider component rather than the game object.
    // Later collision events can identify exactly which collider was touched.
    shapeDef.userData = this;

    const glm::vec2 halfSizeMetres = sizePixels * (0.5f / PhysicsWorld2D::PixelsPerMetre);

    const b2Polygon box = b2MakeBox(halfSizeMetres.x, halfSizeMetres.y);

    shapeId = b2CreatePolygonShape(rigidbody->getBodyId(), &shapeDef, &box);
}

void ColliderComponent::setBoxSizePixels(const glm::vec2 &newSizePixels)
{
    if (!b2Shape_IsValid(shapeId) || rigidbody == nullptr) {
        return;
    }

    sizePixels = newSizePixels;

    const glm::vec2 halfSizeMetres = sizePixels * (0.5f / PhysicsWorld2D::PixelsPerMetre);

    const b2Polygon box = b2MakeBox(halfSizeMetres.x, halfSizeMetres.y);

    b2Shape_SetPolygon(shapeId, &box);

    // Changing polygon geometry does not automatically update dynamic-body mass.
    b2Body_ApplyMassFromShapes(rigidbody->getBodyId());
}

void ColliderComponent::destroyShape()
{
    if (!b2Shape_IsValid(shapeId)) {
        return;
    }

    b2DestroyShape(shapeId, true);
    shapeId = b2_nullShapeId;
}

void ColliderComponent::Update(std::uint64_t)
{
    // Box2D owns collision detection. This component only owns shape setup,
    // configuration and future runtime deformation.
}
