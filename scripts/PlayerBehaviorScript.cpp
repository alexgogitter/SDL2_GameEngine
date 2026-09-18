#include "scriptComponent.hpp"

#include "object.hpp"

class PlayerBehaviorScript final : public ScriptComponent
{
  public:
    explicit PlayerBehaviorScript(Object *owner) : ScriptComponent("PlayerBehaviorScript", owner) {}

  protected:
    void Setup() override {}
    void PreUpdate(std::uint64_t milliseconds) override {}
    void Update(std::uint64_t milliseconds) override {}
    void Destroy() override {}
};

REGISTER_USER_SCRIPT(PlayerBehaviorScript)
