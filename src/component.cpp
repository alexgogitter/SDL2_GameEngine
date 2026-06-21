#include <cstdint>
#include "component.hpp"

void Component::update(std::uint64_t deltaTime)
{
    Update(deltaTime);
}

