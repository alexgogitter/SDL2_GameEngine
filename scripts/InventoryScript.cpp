#include "scriptComponent.hpp"

#include "object.hpp"

class InventoryScript final : public ScriptComponent
{
  public:
    explicit InventoryScript(Object *owner) : ScriptComponent("InventoryScript", owner) {}

  protected:
    void Setup() override {}
    void PreUpdate(std::uint64_t milliseconds) override {}
    void Update(std::uint64_t milliseconds) override {}
    void Destroy() override {}
};

REGISTER_USER_SCRIPT(InventoryScript)
