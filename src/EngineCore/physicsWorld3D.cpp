#include "physicsWorld3D.hpp"

#include <algorithm>
#include <iostream>
#include <memory>

#include <PxPhysicsAPI.h>

#include "physxContext.hpp"

struct PhysicsWorld3D::Impl
{
    physx::PxPhysics *physics = nullptr;

    physx::PxDefaultCpuDispatcher *dispatcher = nullptr;

    physx::PxScene *scene = nullptr;

    bool valid = false;

    explicit Impl(void *physicsHandle) : physics(static_cast<physx::PxPhysics *>(physicsHandle))
    {
        if (physics == nullptr) {
            std::cerr << "[PhysX] Cannot create the 3D "
                         "world without a valid SDK.\n";

            return;
        }

        dispatcher = physx::PxDefaultCpuDispatcherCreate(2);

        if (dispatcher == nullptr) {
            std::cerr << "[PhysX] Failed to create the "
                         "3D CPU dispatcher.\n";

            return;
        }

        physx::PxSceneDesc description(physics->getTolerancesScale());

        // Conventional 3D coordinates:
        // +Y points upward.
        description.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);

        description.cpuDispatcher = dispatcher;

        description.filterShader = physx::PxDefaultSimulationFilterShader;

        // Required later for fast continuous
        // collision bodies.
        description.flags |= physx::PxSceneFlag::eENABLE_CCD;

        if (!description.isValid()) {
            std::cerr << "[PhysX] Invalid 3D scene "
                         "description.\n";

            return;
        }

        scene = physics->createScene(description);

        if (scene == nullptr) {
            std::cerr << "[PhysX] Failed to create the "
                         "3D simulation scene.\n";

            return;
        }

        valid = true;

        std::cout << "[PhysX] 3D simulation scene "
                     "created.\n";
    }

    ~Impl()
    {
        const bool wasValid = valid;
        valid = false;

        if (scene != nullptr) {
            scene->release();
            scene = nullptr;
        }

        if (dispatcher != nullptr) {
            dispatcher->release();
            dispatcher = nullptr;
        }

        physics = nullptr;

        if (wasValid) {
            std::cout << "[PhysX] 3D simulation scene "
                         "destroyed.\n";
        }
    }
};

PhysicsWorld3D::PhysicsWorld3D(PhysXContext &context) : impl(std::make_unique<Impl>(context.getPhysicsHandle())) {}

PhysicsWorld3D::~PhysicsWorld3D() = default;

bool PhysicsWorld3D::isValid() const { return impl != nullptr && impl->valid && impl->scene != nullptr; }

void PhysicsWorld3D::Step(float frameDeltaSeconds)
{
    lastSimulatedSeconds = 0.0f;
    if (!isValid()) {
        return;
    }

    accumulator += std::clamp(frameDeltaSeconds, 0.0f, 0.25f);

    while (accumulator >= FixedTimeStep) {
        impl->scene->simulate(FixedTimeStep);

        impl->scene->fetchResults(true);

        lastSimulatedSeconds += FixedTimeStep;

        accumulator -= FixedTimeStep;
    }
}

void *PhysicsWorld3D::getPhysicsHandle() const { return impl != nullptr ? impl->physics : nullptr; }

void *PhysicsWorld3D::getSceneHandle() const { return impl != nullptr ? impl->scene : nullptr; }

float PhysicsWorld3D::getLastSimulatedSeconds() const { return lastSimulatedSeconds; }