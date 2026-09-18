#include <SDL.h>
#include "logger.hpp"
#include <sstream>
#include "physxContext.hpp"

#include <iostream>
#include <memory>

#include <PxPhysicsAPI.h>

namespace
{
class PhysXErrorCallback final : public physx::PxErrorCallback
{
  public:
    void reportError(physx::PxErrorCode::Enum code, const char *message, const char *file, int line) override { { std::ostringstream diagnostic; diagnostic << "[PhysX] Error " << static_cast<int>(code) << " at " << file << ':' << line << ": " << message << '\n'; Logger::write(LogLevel::Error, "Engine", diagnostic.str()); } }
};
} // namespace

struct PhysXContext::Impl
{
    physx::PxDefaultAllocator allocator;
    PhysXErrorCallback errorCallback;

    physx::PxFoundation *foundation = nullptr;
    physx::PxPhysics *physics = nullptr;

    bool extensionsInitialised = false;
    bool valid = false;

    Impl()
    {
        foundation = PxCreateFoundation(PX_PHYSICS_VERSION, allocator, errorCallback);

        if (foundation == nullptr) {
            { std::ostringstream diagnostic; diagnostic << "[PhysX] Failed to create the foundation.\n"; Logger::write(LogLevel::Error, "Engine", diagnostic.str()); }
            return;
        }

        const physx::PxTolerancesScale scale;

        physics = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, scale, true, nullptr);

        if (physics == nullptr) {
            { std::ostringstream diagnostic; diagnostic << "[PhysX] Failed to create the physics SDK.\n"; Logger::write(LogLevel::Error, "Engine", diagnostic.str()); }
            return;
        }

        extensionsInitialised = PxInitExtensions(*physics, nullptr);

        if (!extensionsInitialised) {
            { std::ostringstream diagnostic; diagnostic << "[PhysX] Failed to initialise extensions.\n"; Logger::write(LogLevel::Error, "Engine", diagnostic.str()); }
            return;
        }

        valid = true;

        { std::ostringstream diagnostic; diagnostic << "[PhysX] Startup complete. Version " << PX_PHYSICS_VERSION_MAJOR << '.' << PX_PHYSICS_VERSION_MINOR << '.' << PX_PHYSICS_VERSION_BUGFIX << '\n'; Logger::write(LogLevel::Info, "Engine", diagnostic.str()); }
    }

    ~Impl()
    {
        if (extensionsInitialised) {
            PxCloseExtensions();
            extensionsInitialised = false;
        }

        if (physics != nullptr) {
            physics->release();
            physics = nullptr;
        }

        if (foundation != nullptr) {
            foundation->release();
            foundation = nullptr;
        }

        if (valid) {
            { std::ostringstream diagnostic; diagnostic << "[PhysX] Shutdown complete.\n"; Logger::write(LogLevel::Info, "Engine", diagnostic.str()); }
        }

        valid = false;
    }
};

PhysXContext::PhysXContext() : impl(std::make_unique<Impl>()) {}

PhysXContext::~PhysXContext() = default;

bool PhysXContext::isValid() const { return impl != nullptr && impl->valid; }

void *PhysXContext::getPhysicsHandle() const
{
    if (impl == nullptr) {
        return nullptr;
    }

    return impl->physics;
}