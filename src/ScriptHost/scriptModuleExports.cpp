#include "scriptComponent.hpp"

#include "componentRegistry.hpp"

#include <vector>

namespace
{
std::vector<ScriptRegistration> &Registrations()
{
    static std::vector<ScriptRegistration> registrations;
    return registrations;
}
} // namespace

void RegisterScriptComponent(ScriptRegistration registration)
{
    if (registration != nullptr) {
        Registrations().push_back(registration);
    }
}

SCRIPT_EXPORT unsigned GetScriptApiVersion() { return EngineScriptApiVersion; }

SCRIPT_EXPORT bool RegisterGameComponents(ComponentRegistry *registry)
{
    if (registry == nullptr) {
        return false;
    }

    bool result = true;

    for (const ScriptRegistration registration : Registrations()) {
        result = registration(registry) && result;
    }

    return result;
}
