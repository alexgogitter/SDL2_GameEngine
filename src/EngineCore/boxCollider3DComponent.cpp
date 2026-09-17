#include "boxCollider3DComponent.hpp"

#include <cstring>
#include <memory>

#include <common.hpp>

#include <PxPhysicsAPI.h>

#include "object.hpp"
#include "rigidbody3DComponent.hpp"

namespace
{
physx::PxVec3 ToPhysX(const glm::vec3 &value) { return {value.x, value.y, value.z}; }

physx::PxQuat ToPhysX(const glm::quat &value) { return {value.x, value.y, value.z, value.w}; }
} // namespace

struct BoxCollider3DComponent::Impl
{
    physx::PxShape *shape = nullptr;
    physx::PxMaterial *material = nullptr;

    // Non-null only when this collider represents a
    // static object without Rigidbody3D.
    physx::PxRigidStatic *ownedStaticActor = nullptr;

    ~Impl()
    {
        if (shape != nullptr) {
            if (physx::PxRigidActor *actor = shape->getActor()) {
                actor->detachShape(*shape);
            }

            shape->release();
            shape = nullptr;
        }

        if (ownedStaticActor != nullptr) {
            ownedStaticActor->release();
            ownedStaticActor = nullptr;
        }

        if (material != nullptr) {
            material->release();
            material = nullptr;
        }
    }
};

BoxCollider3DComponent::BoxCollider3DComponent(Object *parentObject, PhysicsWorld3D &world, Rigidbody3DComponent *rigidbody) : Component("BoxCollider3D", parentObject), impl(std::make_unique<Impl>())
{
    if (parent == nullptr) {
        return;
    }

    auto *physics = static_cast<physx::PxPhysics *>(world.getPhysicsHandle());

    auto *scene = static_cast<physx::PxScene *>(world.getSceneHandle());

    if (physics == nullptr || scene == nullptr) {
        return;
    }

    physx::PxRigidActor *actor = nullptr;

    if (rigidbody != nullptr) {
        actor = static_cast<physx::PxRigidDynamic *>(rigidbody->getActorHandle());
    }
    else {
        const physx::PxTransform pose(ToPhysX(parent->getWorldPosition()), ToPhysX(parent->getWorldRotation()));

        impl->ownedStaticActor = physics->createRigidStatic(pose);

        if (impl->ownedStaticActor == nullptr) {
            return;
        }

        scene->addActor(*impl->ownedStaticActor);

        actor = impl->ownedStaticActor;
    }

    if (actor == nullptr) {
        return;
    }

    impl->material = physics->createMaterial(0.6f, 0.6f, 0.0f);

    if (impl->material == nullptr) {
        return;
    }

    physx::PxShape *shape = physics->createShape(physx::PxBoxGeometry(0.5f, 0.5f, 0.5f), *impl->material, true);

    if (shape == nullptr) {
        return;
    }

    if (!actor->attachShape(*shape)) {
        shape->release();
        return;
    }

    impl->shape = shape;

    updateGeometry();
    updateMaterial();
    updateShapeFlags();
}

BoxCollider3DComponent::~BoxCollider3DComponent() = default;

bool BoxCollider3DComponent::isValid() const { return impl != nullptr && impl->shape != nullptr && impl->shape->getActor() != nullptr; }

std::size_t BoxCollider3DComponent::GetPropertyCount() const { return 6; }

bool BoxCollider3DComponent::GetProperty(std::size_t index, ComponentProperty &property)
{
    switch (index) {
    case 0:
        property.key = "size";
        property.displayName = "Size";
        property.type = ComponentPropertyType::Vector3;
        property.value = &size;
        property.hasRange = true;
        property.minimum = 0.001f;
        property.maximum = 10000.0f;
        property.step = 0.05f;
        return true;

    case 1:
        property.key = "center";
        property.displayName = "Center";
        property.type = ComponentPropertyType::Vector3;
        property.value = &center;
        property.step = 0.05f;
        return true;

    case 2:
        property.key = "staticFriction";
        property.displayName = "Static Friction";
        property.type = ComponentPropertyType::Float;
        property.value = &staticFriction;
        property.hasRange = true;
        property.minimum = 0.0f;
        property.maximum = 1.0f;
        property.step = 0.01f;
        return true;

    case 3:
        property.key = "dynamicFriction";
        property.displayName = "Dynamic Friction";
        property.type = ComponentPropertyType::Float;
        property.value = &dynamicFriction;
        property.hasRange = true;
        property.minimum = 0.0f;
        property.maximum = 1.0f;
        property.step = 0.01f;
        return true;

    case 4:
        property.key = "restitution";
        property.displayName = "Restitution";
        property.type = ComponentPropertyType::Float;
        property.value = &restitution;
        property.hasRange = true;
        property.minimum = 0.0f;
        property.maximum = 1.0f;
        property.step = 0.01f;
        return true;

    case 5:
        property.key = "isTrigger";
        property.displayName = "Is Trigger";
        property.type = ComponentPropertyType::Boolean;
        property.value = &isTrigger;
        return true;

    default:
        return false;
    }
}

void BoxCollider3DComponent::OnPropertyChanged(const char *key)
{
    if (key == nullptr || !isValid()) {
        return;
    }

    if (std::strcmp(key, "size") == 0) {
        size = glm::max(glm::abs(size), glm::vec3(0.001f));

        updateGeometry();
    }
    else if (std::strcmp(key, "center") == 0) {
        updateGeometry();
    }
    else if (std::strcmp(key, "staticFriction") == 0 || std::strcmp(key, "dynamicFriction") == 0 || std::strcmp(key, "restitution") == 0) {
        staticFriction = glm::clamp(staticFriction, 0.0f, 1.0f);

        dynamicFriction = glm::clamp(dynamicFriction, 0.0f, 1.0f);

        restitution = glm::clamp(restitution, 0.0f, 1.0f);

        updateMaterial();
    }
    else if (std::strcmp(key, "isTrigger") == 0) {
        updateShapeFlags();
    }
}

void BoxCollider3DComponent::OnTransformChanged()
{
    if (impl != nullptr && impl->ownedStaticActor != nullptr && parent != nullptr) {
        impl->ownedStaticActor->setGlobalPose(physx::PxTransform(ToPhysX(parent->getWorldPosition()), ToPhysX(parent->getWorldRotation())));
    }

    updateGeometry();
}

void BoxCollider3DComponent::updateGeometry()
{
    if (!isValid() || parent == nullptr) {
        return;
    }

    const glm::vec3 worldScale = glm::abs(parent->getWorldTransform().getScale());

    const glm::vec3 halfExtents = glm::max(glm::abs(size) * worldScale * 0.5f, glm::vec3(0.001f));

    impl->shape->setGeometry(physx::PxBoxGeometry(ToPhysX(halfExtents)));

    const glm::vec3 scaledCenter = center * worldScale;

    impl->shape->setLocalPose(physx::PxTransform(ToPhysX(scaledCenter)));

    physx::PxRigidActor *actor = impl->shape->getActor();

    if (actor == nullptr) {
        return;
    }

    updateMassProperties();
}

void BoxCollider3DComponent::updateMaterial()
{
    if (impl == nullptr || impl->material == nullptr) {
        return;
    }

    impl->material->setStaticFriction(staticFriction);

    impl->material->setDynamicFriction(dynamicFriction);

    impl->material->setRestitution(restitution);
}

void BoxCollider3DComponent::updateShapeFlags()
{
    if (!isValid() || !isEnabled()) {
        return;
    }

    physx::PxShapeFlags flags = physx::PxShapeFlag::eSCENE_QUERY_SHAPE;

    if (isTrigger) {
        flags |= physx::PxShapeFlag::eTRIGGER_SHAPE;
    }
    else {
        flags |= physx::PxShapeFlag::eSIMULATION_SHAPE;
    }

    impl->shape->setFlags(flags);

    if (!isTrigger) {
        updateMassProperties();
    }
}

void BoxCollider3DComponent::updateMassProperties()
{
    if (!isValid() || !isEnabled() || isTrigger) {
        return;
    }

    physx::PxRigidActor *actor = impl->shape->getActor();

    if (actor == nullptr) {
        return;
    }

    if (physx::PxRigidDynamic *dynamicBody = actor->is<physx::PxRigidDynamic>()) {
        physx::PxRigidBodyExt::setMassAndUpdateInertia(*dynamicBody, dynamicBody->getMass());
    }
}
void BoxCollider3DComponent::OnEnable() { updateShapeFlags(); }

void BoxCollider3DComponent::OnDisable()
{
    if (!isValid()) {
        return;
    }

    impl->shape->setFlags(physx::PxShapeFlags{});
}
