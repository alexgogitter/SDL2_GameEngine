#ifndef COLLIDER_COMPONENT_H
#define COLLIDER_COMPONENT_H
#include <cstdint>
#include <SDL.h>
#include "component.hpp"
#include "EventListener.hpp"
#include "glm/glm.hpp"
#include "object.hpp"

class ColliderComponent : public Component
{
public:
    ColliderComponent(Object* _parent, const SDL_Rect& rect) : Component("ColliderComponent", _parent), collider_box(rect) {}
    void Update(std::uint64_t deltaTime) override;
    SDL_Rect& get_Collider() {return collider_box;};

private:
    SDL_Rect collider_box;
};

#endif // COLLIDER_COMPONENT_H