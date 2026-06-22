#include "component.hpp"

void Component::update(std::uint64_t deltaTime)
{
    Update(deltaTime);
}

void Component::draw(Renderer* renderer)
{
    Draw(renderer);
}
