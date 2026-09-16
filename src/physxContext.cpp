#include "physxContext.hpp"

#include <iostream>
#include <memory>

#include <PxPhysicsAPI.h>

namespace
{
    class PhysXErrorCallback final
        : public physx::PxErrorCallback
    {
    public:
        void reportError(
            physx::PxErrorCode::Enum code,
            const char* message,
            const char* file,
            int line
        ) override
        {
            std::cerr
                << "[PhysX] Error "
                << static_cast<int>(code)
                << " at "
                << file
                << ':'
                << line
                << ": "
                << message
                << '\n';
        }
    };
}

struct PhysXContext::Impl
{
    physx::PxDefaultAllocator allocator;
    PhysXErrorCallback errorCallback;

    physx::PxFoundation* foundation = nullptr;
    physx::PxPhysics* physics = nullptr;

    bool extensionsInitialised = false;
    bool valid = false;

    Impl()
    {
        foundation = PxCreateFoundation(
            PX_PHYSICS_VERSION,
            allocator,
            errorCallback
        );

        if (foundation == nullptr)
        {
            std::cerr
                << "[PhysX] Failed to create the foundation.\n";
            return;
        }

        const physx::PxTolerancesScale scale;

        physics = PxCreatePhysics(
            PX_PHYSICS_VERSION,
            *foundation,
            scale,
            true,
            nullptr
        );

        if (physics == nullptr)
        {
            std::cerr
                << "[PhysX] Failed to create the physics SDK.\n";
            return;
        }

        extensionsInitialised =
            PxInitExtensions(*physics, nullptr);

        if (!extensionsInitialised)
        {
            std::cerr
                << "[PhysX] Failed to initialise extensions.\n";
            return;
        }

        valid = true;

        std::cout
            << "[PhysX] Startup complete. Version "
            << PX_PHYSICS_VERSION_MAJOR
            << '.'
            << PX_PHYSICS_VERSION_MINOR
            << '.'
            << PX_PHYSICS_VERSION_BUGFIX
            << '\n';
    }

    ~Impl()
    {
        if (extensionsInitialised)
        {
            PxCloseExtensions();
            extensionsInitialised = false;
        }

        if (physics != nullptr)
        {
            physics->release();
            physics = nullptr;
        }

        if (foundation != nullptr)
        {
            foundation->release();
            foundation = nullptr;
        }

        if (valid)
        {
            std::cout
                << "[PhysX] Shutdown complete.\n";
        }

        valid = false;
    }
};

PhysXContext::PhysXContext()
    : impl(std::make_unique<Impl>())
{
}

PhysXContext::~PhysXContext() = default;

bool PhysXContext::isValid() const
{
    return impl != nullptr && impl->valid;
}

void* PhysXContext::getPhysicsHandle() const
{
    if (impl == nullptr)
    {
        return nullptr;
    }

    return impl->physics;
}