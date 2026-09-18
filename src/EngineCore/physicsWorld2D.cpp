#include <SDL.h>
#include "logger.hpp"
#include <sstream>
#include "physicsWorld2D.hpp"

#include <algorithm>
#include <iostream>
#include <memory>

#include <PxPhysicsAPI.h>

#include "physxContext.hpp"

struct PhysicsWorld2D::PhysXState
{
    physx::PxDefaultCpuDispatcher *dispatcher = nullptr;
    physx::PxScene *scene = nullptr;

    bool valid = false;

    explicit PhysXState(void *physicsHandle)
    {
        auto *physics = static_cast<physx::PxPhysics *>(physicsHandle);

        if (physics == nullptr) {
            { std::ostringstream message; message << "[PhysX] Cannot create scene without "
                         "a valid physics SDK.\n"; Logger::write(LogLevel::Error, "Engine", message.str()); }
            return;
        }

        dispatcher = physx::PxDefaultCpuDispatcherCreate(2);

        if (dispatcher == nullptr) {
            { std::ostringstream message; message << "[PhysX] Failed to create CPU dispatcher.\n"; Logger::write(LogLevel::Error, "Engine", message.str()); }
            return;
        }

        physx::PxSceneDesc sceneDescription(physics->getTolerancesScale());

        // The engine uses X/Y for 2D motion and positive Y downward.
        sceneDescription.gravity = physx::PxVec3(0.0f, 9.81f, 0.0f);

        sceneDescription.cpuDispatcher = dispatcher;
        sceneDescription.filterShader = physx::PxDefaultSimulationFilterShader;

        if (!sceneDescription.isValid()) {
            { std::ostringstream message; message << "[PhysX] The 2D scene description is invalid.\n"; Logger::write(LogLevel::Error, "Engine", message.str()); }
            return;
        }

        scene = physics->createScene(sceneDescription);

        if (scene == nullptr) {
            { std::ostringstream message; message << "[PhysX] Failed to create the 2D scene.\n"; Logger::write(LogLevel::Error, "Engine", message.str()); }
            return;
        }

        valid = true;

        { std::ostringstream message; message << "[PhysX] 2D simulation scene created.\n"; Logger::write(LogLevel::Info, "Engine", message.str()); }
    }

    ~PhysXState()
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

        if (wasValid) {
            { std::ostringstream message; message << "[PhysX] 2D simulation scene destroyed.\n"; Logger::write(LogLevel::Info, "Engine", message.str()); }
        }
    }
};

PhysicsWorld2D::PhysicsWorld2D(PhysXContext &physxContext) : physxState(std::make_unique<PhysXState>(physxContext.getPhysicsHandle()))
{
    // Temporary legacy world. It remains alive until the existing
    // Rigidbody2D and BoxCollider2D implementations are migrated.
    b2WorldDef worldDefinition = b2DefaultWorldDef();
    worldDefinition.gravity = {0.0f, 9.8f};

    worldId = b2CreateWorld(&worldDefinition);
}

PhysicsWorld2D::~PhysicsWorld2D()
{
    if (b2World_IsValid(worldId)) {
        b2DestroyWorld(worldId);
    }

    worldId = b2_nullWorldId;
}

void PhysicsWorld2D::Step(float frameDeltaSeconds)
{
    const float clampedDelta = std::clamp(frameDeltaSeconds, 0.0f, 0.25f);

    accumulator += clampedDelta;

    while (accumulator >= FixedTimeStep) {
        if (physxState != nullptr && physxState->scene != nullptr) {
            physxState->scene->simulate(FixedTimeStep);
            physxState->scene->fetchResults(true);
        }

        // Temporary until all bodies and colliders use PhysX.
        if (b2World_IsValid(worldId)) {
            b2World_Step(worldId, FixedTimeStep, SubSteps);
        }

        accumulator -= FixedTimeStep;
    }
}

bool PhysicsWorld2D::isValid() const { return physxState != nullptr && physxState->valid && physxState->scene != nullptr; }

b2Vec2 PhysicsWorld2D::pixelsToMetres(const glm::vec2 &pixelPosition) { return {pixelPosition.x / PixelsPerMetre, pixelPosition.y / PixelsPerMetre}; }

glm::vec2 PhysicsWorld2D::metresToPixels(const b2Vec2 &worldPosition) { return {worldPosition.x * PixelsPerMetre, worldPosition.y * PixelsPerMetre}; }