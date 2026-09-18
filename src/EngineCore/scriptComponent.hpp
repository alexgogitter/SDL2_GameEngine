#pragma once
#include "component.hpp"
#include "componentRegistry.hpp"

using ScriptRegistration = bool (*)(ComponentRegistry *registry);

/// Called by REGISTER_USER_SCRIPT during module initialization. The script host
/// owns the registration list and exports it to the editor/player.
void RegisterScriptComponent(ScriptRegistration registration);
/// Base for user C++ scripts. Setup runs once before the first PreUpdate.
/// PreUpdate/Update receive elapsed milliseconds. Destroy runs while the DLL is loaded.
class ScriptComponent : public Component
{
protected:
    virtual void Destroy() {}
    void OnDestroy() final { Destroy(); }
public:
    ScriptComponent(const std::string &type, Object *owner) : Component(type, owner) {}
};
/// Increment when changing the engine/script binary contract. Use matching toolchain and configuration.
inline constexpr unsigned EngineScriptApiVersion = 1;
#ifdef _WIN32
#define SCRIPT_EXPORT extern "C" __declspec(dllexport)
#else
#define SCRIPT_EXPORT extern "C" __attribute__((visibility("default")))
#endif

/// Put this once after a user ScriptComponent class declaration. The class name
/// becomes its component name and the script is added under "User Scripts".
#define REGISTER_USER_SCRIPT(ScriptType)                                                                                         \
    namespace                                                                                                                    \
    {                                                                                                                            \
    Component *Create##ScriptType(Object *owner, const ComponentCreateContext &) { return new ScriptType(owner); }             \
    bool Register##ScriptType(ComponentRegistry *registry)                                                                       \
    {                                                                                                                            \
        return registry != nullptr && registry->registerType(#ScriptType, #ScriptType, "User Scripts", &Create##ScriptType,  \
                                                           nullptr, 0);                                                        \
    }                                                                                                                            \
    struct ScriptType##AutoRegistration                                                                                          \
    {                                                                                                                            \
        ScriptType##AutoRegistration() { RegisterScriptComponent(&Register##ScriptType); }                                     \
    };                                                                                                                           \
    const ScriptType##AutoRegistration scriptType##AutoRegistration;                                                            \
    }
