#include "scriptComponent.hpp"

#include "object.hpp"

/// Example script. Add it from Inspector > Add Component > User Scripts.
class SpinScript final : public ScriptComponent
{
    float degreesPerSecond = 45.0f;

  public:
    explicit SpinScript(Object *owner) : ScriptComponent("SpinScript", owner) {}

  protected:
    void Setup() override {}
    void PreUpdate(std::uint64_t) override {}

    void Update(std::uint64_t milliseconds) override
    {
        Object *object = getParent();
        const float radians = glm::radians(degreesPerSecond) * static_cast<float>(milliseconds) / 1000.0f;
        object->transform.rotateBy(glm::angleAxis(radians, glm::vec3(0.0f, 1.0f, 0.0f)));

        // This sample intentionally does not teleport a Rigidbody3D every
        // frame. Dynamic physics bodies must be driven through physics APIs.
    }

    void Destroy() override {}

    std::size_t GetPropertyCount() const override { return 1; }

    bool GetProperty(std::size_t index, ComponentProperty &property) override
    {
        if (index != 0) {
            return false;
        }

        property = {"degreesPerSecond", "Degrees per second", ComponentPropertyType::Float, &degreesPerSecond};
        return true;
    }
};

REGISTER_USER_SCRIPT(SpinScript)
