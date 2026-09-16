#include "rigidbody3DComponent.hpp"

#include <algorithm>
#include <cstring>

#include <glm/gtc/quaternion.hpp>

#include <PxPhysicsAPI.h>

#include "object.hpp"
#include "physicsWorld3D.hpp"

namespace
{
    physx::PxVec3 ToPhysX(
        const glm::vec3& value
    )
    {
        return {
            value.x,
            value.y,
            value.z
        };
    }

    physx::PxQuat ToPhysX(
        const glm::quat& value
    )
    {
        return {
            value.x,
            value.y,
            value.z,
            value.w
        };
    }

    glm::vec3 FromPhysX(
        const physx::PxVec3& value
    )
    {
        return {
            value.x,
            value.y,
            value.z
        };
    }

    glm::quat FromPhysX(
        const physx::PxQuat& value
    )
    {
        return glm::quat(
            value.w,
            value.x,
            value.y,
            value.z
        );
    }
}

struct Rigidbody3DComponent::Impl
{
    physx::PxRigidDynamic* actor = nullptr;

    ~Impl()
    {
        if (actor != nullptr)
        {
            actor->release();
            actor = nullptr;
        }
    }
};

Rigidbody3DComponent::Rigidbody3DComponent(
    Object* parentObject,
    PhysicsWorld3D& world
)
    : Component(
        "Rigidbody3D",
        parentObject
      ),
      impl(std::make_unique<Impl>()),
      physicsWorld(&world)
{
    if (parent == nullptr)
    {
        return;
    }

    physicsRotationDegrees =
    glm::degrees(
        glm::eulerAngles(
            parent->getWorldRotation()
        )
    );

    auto* physics =
        static_cast<physx::PxPhysics*>(
            world.getPhysicsHandle()
        );

    auto* scene =
        static_cast<physx::PxScene*>(
            world.getSceneHandle()
        );

    if (physics == nullptr || scene == nullptr)
    {
        return;
    }

    const physx::PxTransform initialPose(
        ToPhysX(parent->getWorldPosition()),
        ToPhysX(parent->getWorldRotation())
    );

    impl->actor =
        physics->createRigidDynamic(initialPose);

    if (impl->actor == nullptr)
    {
        return;
    }

    impl->actor->userData = parent;
    impl->actor->setMass(mass);

    impl->actor->setActorFlag(
        physx::PxActorFlag::eDISABLE_GRAVITY,
        !useGravity
    );

    impl->actor->setRigidBodyFlag(
        physx::PxRigidBodyFlag::eKINEMATIC,
        isKinematic
    );

    impl->actor->setLinearDamping(
        linearDamping
    );

    impl->actor->setAngularDamping(
        angularDamping
    );

    scene->addActor(*impl->actor);
}

Rigidbody3DComponent::~Rigidbody3DComponent() =
    default;

bool Rigidbody3DComponent::isValid() const
{
    return
        impl != nullptr &&
        impl->actor != nullptr;
}

void Rigidbody3DComponent::Update(std::uint64_t)
{
    if (!isValid() || parent == nullptr)
    {
        return;
    }

    const physx::PxTransform pose =
        impl->actor->getGlobalPose();

    parent->transform.setPosition(
        FromPhysX(pose.p)
    );

    const glm::quat currentRotation =
    FromPhysX(pose.q);

    parent->transform.setRotation(
        currentRotation
    );

    physicsRotationDegrees =
        glm::degrees(
            glm::eulerAngles(
                currentRotation
            )
        );

    angularVelocityDegrees =
        glm::degrees(
            FromPhysX(
                impl->actor->
                    getAngularVelocity()
            )
        );

    const glm::vec3 currentVelocity =
        FromPhysX(
            impl->actor->getLinearVelocity()
        );

    const float simulatedSeconds =
    physicsWorld != nullptr
        ? physicsWorld->
            getLastSimulatedSeconds()
        : 0.0f;

    if (
        hasPreviousLinearVelocity &&
        simulatedSeconds > 0.000001f
    )
    {
        measuredAcceleration =
            (
                currentVelocity -
                previousLinearVelocity
            ) / simulatedSeconds;
    }
    linearVelocity = currentVelocity;
    previousLinearVelocity =
        currentVelocity;

    hasPreviousLinearVelocity = true;
}

std::size_t
Rigidbody3DComponent::GetPropertyCount() const
{
    return 9;
}

bool Rigidbody3DComponent::GetProperty(
    std::size_t index,
    ComponentProperty& property
)
{
    switch (index)
    {
        case 0:
            property.key = "mass";
            property.displayName = "Mass";
            property.type =
                ComponentPropertyType::Float;
            property.value = &mass;
            property.hasRange = true;
            property.minimum = 0.001f;
            property.maximum = 10000.0f;
            property.step = 0.1f;
            return true;

        case 1:
            property.key = "useGravity";
            property.displayName = "Use Gravity";
            property.type =
                ComponentPropertyType::Boolean;
            property.value = &useGravity;
            return true;

        case 2:
            property.key = "isKinematic";
            property.displayName = "Is Kinematic";
            property.type =
                ComponentPropertyType::Boolean;
            property.value = &isKinematic;
            return true;

        case 3:
            property.key = "linearVelocity";
            property.displayName = "Linear Velocity";

            property.type = ComponentPropertyType::Vector3;

            property.value = &linearVelocity;

            // PhysX does not accept velocity changes while
            // the body is kinematic.
            property.readOnly = isKinematic;

            // Runtime state should not become a saved
            // project default.
            
            property.step = 0.1f;
            return true;

        case 4:
            property.key = "measuredAcceleration";
            property.displayName =
                "Measured Acceleration";

            property.type = ComponentPropertyType::Vector3;

            property.value = &measuredAcceleration;

            property.readOnly = true;
            property.serializable = false;
            return true;

        case 5:
            property.key = "physicsRotation";
            property.displayName =
                "Physics Rotation";

            property.type =
                ComponentPropertyType::Vector3;

            property.value =
                &physicsRotationDegrees;

            // Transform already owns saved rotation.
            property.serializable = false;
            property.step = 0.5f;
        return true;

        case 6:
            property.key = "angularVelocity";
            property.displayName =
                "Angular Velocity";

            property.type =
                ComponentPropertyType::Vector3;

            property.value =
                &angularVelocityDegrees;

            property.readOnly =
                isKinematic;

            property.step = 1.0f;
        return true;

        case 7:
            property.key = "linearDamping";
            property.displayName =
                "Linear Damping";

            property.type =
                ComponentPropertyType::Float;

            property.value =
                &linearDamping;

            property.hasRange = true;
            property.minimum = 0.0f;
            property.maximum = 100.0f;
            property.step = 0.01f;
            return true;

        case 8:
            property.key = "angularDamping";
            property.displayName =
                "Angular Damping";

            property.type =
                ComponentPropertyType::Float;

            property.value =
                &angularDamping;

            property.hasRange = true;
            property.minimum = 0.0f;
            property.maximum = 100.0f;
            property.step = 0.01f;
            return true;

        default:
            return false;
    }
}

void Rigidbody3DComponent::OnPropertyChanged(
    const char* key
)
{
    if (!isValid() || key == nullptr)
    {
        return;
    }

    if (std::strcmp(key, "mass") == 0)
    {
        mass = std::max(mass, 0.001f);
        impl->actor->setMass(mass);
    }
    else if (
        std::strcmp(key, "useGravity") == 0
    )
    {
        impl->actor->setActorFlag(
            physx::PxActorFlag::eDISABLE_GRAVITY,
            !useGravity
        );
    }
    else if (std::strcmp(key, "linearVelocity") == 0)
    {
        if (isKinematic)
        {
            linearVelocity = glm::vec3(0.0f);
            return;
        }

        impl->actor->setLinearVelocity(ToPhysX(linearVelocity), true);

        previousLinearVelocity = linearVelocity;

        measuredAcceleration = glm::vec3(0.0f);

        hasPreviousLinearVelocity = true;
    }
    
    else if (
    std::strcmp(key, "physicsRotation") == 0)
    {
        const glm::quat newRotation =
            glm::normalize(
                glm::quat(
                    glm::radians(
                        physicsRotationDegrees
                    )
                )
            );

        physx::PxTransform pose =
            impl->actor->getGlobalPose();

        pose.q = ToPhysX(newRotation);

        impl->actor->setGlobalPose(
            pose,
            true
        );

        const physx::PxVec3 zeroVelocity(
            0.0f,
            0.0f,
            0.0f
        );

        // An Inspector rotation acts as an angular teleport.
        if (!isKinematic)
        {
            impl->actor->setAngularVelocity(
                zeroVelocity
            );
        }

        angularVelocityDegrees =
            glm::vec3(0.0f);

        parent->transform.setRotation(
            newRotation
        );
    }
    else if (
        std::strcmp(
            key,
            "angularVelocity"
        ) == 0
    )
    {
        if (isKinematic)
        {
            angularVelocityDegrees =
                glm::vec3(0.0f);

            return;
        }

        impl->actor->setAngularVelocity(
            ToPhysX(
                glm::radians(
                    angularVelocityDegrees
                )
            ),
            true
        );
    }
    else if (
    std::strcmp(
        key,
        "linearDamping"
        ) == 0
    )
    {
        linearDamping = std::max(linearDamping, 0.0f);

        impl->actor->setLinearDamping(linearDamping);
    }
    else if (std::strcmp(key, "angularDamping") == 0)
    {
        angularDamping = std::max(angularDamping, 0.0f);

        impl->actor->setAngularDamping(
            angularDamping
        );
    }
    else if (std::strcmp(key, "isKinematic") == 0)
    {
        const physx::PxVec3 zeroVelocity(
            0.0f,
            0.0f,
            0.0f
        );

        const physx::PxTransform displayedPose(
            ToPhysX(parent->getWorldPosition()),
            ToPhysX(parent->getWorldRotation())
        );

        if (isKinematic)
        {
            // Clear dynamic momentum before changing modes.
            impl->actor->setLinearVelocity(
                zeroVelocity
            );

            impl->actor->setAngularVelocity(
                zeroVelocity
            );

            impl->actor->setGlobalPose(
                displayedPose,
                true
            );

            impl->actor->setRigidBodyFlag(
                physx::PxRigidBodyFlag::eKINEMATIC,
                true
            );
        }
        else
        {
            impl->actor->setRigidBodyFlag(
                physx::PxRigidBodyFlag::eKINEMATIC,
                false
            );

            impl->actor->setGlobalPose(
                displayedPose,
                true
            );

            impl->actor->setLinearVelocity(
                zeroVelocity
            );

            impl->actor->setAngularVelocity(
                zeroVelocity
            );

            impl->actor->wakeUp();
        }

        // This section runs after either mode transition.
        // It resets the values displayed by the Inspector.
        linearVelocity = glm::vec3(0.0f);

        previousLinearVelocity = glm::vec3(0.0f);

        measuredAcceleration = glm::vec3(0.0f);

        angularVelocityDegrees = glm::vec3(0.0f);

        hasPreviousLinearVelocity = false;

    }
}

void Rigidbody3DComponent::OnTransformChanged()
{
    if (!isValid() || parent == nullptr)
    {
        return;
    }

    const physx::PxTransform targetPose(
        ToPhysX(parent->getWorldPosition()),
        ToPhysX(parent->getWorldRotation())
    );

    physicsRotationDegrees =
        glm::degrees(
            glm::eulerAngles(
                parent->getWorldRotation()
            )
        );

    angularVelocityDegrees =
        glm::vec3(0.0f);

    // Inspector transform edits are teleports, not kinematic
    // movement targets.
    impl->actor->setGlobalPose(
        targetPose,
        true
    );

    // An editor teleport acts as a reset. Otherwise an old
    // velocity could immediately return the object to its
    // previous position.
    if (!isKinematic)
    {
        const physx::PxVec3 zeroVelocity(
            0.0f,
            0.0f,
            0.0f
        );

        impl->actor->setLinearVelocity(
            zeroVelocity
        );

        impl->actor->setAngularVelocity(
            zeroVelocity
        );
    }
}

void* Rigidbody3DComponent::getActorHandle() const
{
    return isValid()
        ? impl->actor
        : nullptr;
}

void Rigidbody3DComponent::OnEnable()
{
    if (!isValid())
    {
        return;
    }

    impl->actor->setActorFlag(
        physx::PxActorFlag::eDISABLE_SIMULATION,
        false
    );

    hasPreviousLinearVelocity = false;
}

void Rigidbody3DComponent::OnDisable()
{
    if (!isValid())
    {
        return;
    }

    impl->actor->setActorFlag(
        physx::PxActorFlag::eDISABLE_SIMULATION,
        true
    );

    measuredAcceleration =
        glm::vec3(0.0f);

    hasPreviousLinearVelocity = false;
}