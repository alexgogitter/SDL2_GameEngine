#include "rigidbodyComponent.hpp"

#include "object.hpp"

RigidbodyComponent::RigidbodyComponent(Object *parentObject, PhysicsWorld2D &world, BodyType2D bodyType, bool fixedRotation, float gravityScale, bool isBullet) : Component("Rigidbody2D", parentObject), physicsWorld(&world)
{
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = toBox2DBodyType(bodyType);
    bodyDef.position = PhysicsWorld2D::pixelsToMetres(parent->transform.getPosition());
    bodyDef.fixedRotation = fixedRotation;
    bodyDef.gravityScale = gravityScale;
    bodyDef.isBullet = isBullet;

    // Lets future physics events map back to the engine object.
    bodyDef.userData = parent;

    bodyId = b2CreateBody(physicsWorld->getWorldId(), &bodyDef);
}

RigidbodyComponent::~RigidbodyComponent()
{
    if (physicsWorld != nullptr && b2World_IsValid(physicsWorld->getWorldId()) && b2Body_IsValid(bodyId)) {
        // Attached Box2D shapes are destroyed automatically with the body.
        b2DestroyBody(bodyId);
    }

    bodyId = b2_nullBodyId;
}

void RigidbodyComponent::setLinearVelocityPixels(const glm::vec2 &pixelsPerSecond)
{
    if (!b2Body_IsValid(bodyId)) {
        return;
    }

    b2Body_SetLinearVelocity(bodyId, PhysicsWorld2D::pixelsToMetres(pixelsPerSecond));
}

glm::vec2 RigidbodyComponent::getLinearVelocityPixels() const
{
    if (!b2Body_IsValid(bodyId)) {
        return {0.0f, 0.0f};
    }

    return PhysicsWorld2D::metresToPixels(b2Body_GetLinearVelocity(bodyId));
}

void RigidbodyComponent::applyImpulsePixels(const glm::vec2 &impulse)
{
    if (!b2Body_IsValid(bodyId)) {
        return;
    }

    b2Body_ApplyLinearImpulseToCenter(bodyId, PhysicsWorld2D::pixelsToMetres(impulse), true);
}

void RigidbodyComponent::teleportPixels(const glm::vec2 &pixelPosition, float angleRadians)
{
    if (!b2Body_IsValid(bodyId)) {
        return;
    }

    b2Body_SetTransform(bodyId, PhysicsWorld2D::pixelsToMetres(pixelPosition), b2MakeRot(angleRadians));
}

void RigidbodyComponent::Update(std::uint64_t)
{
    if (!b2Body_IsValid(bodyId)) {
        return;
    }

    parent->transform.setPosition(PhysicsWorld2D::metresToPixels(b2Body_GetPosition(bodyId)));

    parent->transform.setRotation(b2Rot_GetAngle(b2Body_GetRotation(bodyId)));
}

b2BodyType RigidbodyComponent::toBox2DBodyType(BodyType2D bodyType)
{
    switch (bodyType) {
    case BodyType2D::Static:
        return b2_staticBody;

    case BodyType2D::Kinematic:
        return b2_kinematicBody;

    case BodyType2D::Dynamic:
        return b2_dynamicBody;
    }

    return b2_staticBody;
}
