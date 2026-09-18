#include "scriptComponent.hpp"
#include "componentRegistry.hpp"
#include "object.hpp"
// Deliberately different layout and behavior to verify reconstruction across DLL versions.
class ReloadFixture final : public ScriptComponent
{
    glm::vec4 newPrivateState{1,2,3,4};
    float speed = 99;
protected:
    void Update(std::uint64_t ms) override
    {
        getParent()->transform.translate(glm::vec3(speed * static_cast<float>(ms) / 1000.0f,0,0));
    }
    std::size_t GetPropertyCount() const override { return 1; }
    bool GetProperty(std::size_t i, ComponentProperty &p) override
    {
        if (i != 0) return false;
        p = {"degreesPerSecond", "Speed", ComponentPropertyType::Float, &speed}; return true;
    }
public:
    ReloadFixture(Object *o) : ScriptComponent("SpinScript",o) {}
};
Component *CreateReloadFixture(Object *o, const ComponentCreateContext &) { return new ReloadFixture(o); }
SCRIPT_EXPORT unsigned GetScriptApiVersion()
{
#ifdef INCOMPATIBLE_SCRIPT_FIXTURE
    return EngineScriptApiVersion + 1;
#else
    return EngineScriptApiVersion;
#endif
}
SCRIPT_EXPORT bool RegisterGameComponents(ComponentRegistry *r)
{
    return r->registerType("SpinScript","Spin Script","User Scripts",CreateReloadFixture,nullptr,0);
}
