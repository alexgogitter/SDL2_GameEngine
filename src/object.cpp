#include "object.hpp"


void Object::update(std::uint64_t deltaTime)
{
    Update(deltaTime);
    for (const auto& pair : components)
    {
        Component* c = pair.second;
        c->update(deltaTime);
    }
    for (Object* o : child_Objects)
    {
        o->update(deltaTime);
    }
}

void Object::draw(Renderer* r)
{
    //SDL_RenderCopy(r, tex, NULL, &collider_box);
    if (r == nullptr)
    {
        fprintf(stderr, "ERROR: Renderer is null in Object::draw().\n");
        return;
    }
    SDL_Rect dest_rect;
    dest_rect.w = static_cast<int>(entity_box.w * transform.getScale().x);
    dest_rect.h = static_cast<int>(entity_box.h * transform.getScale().y);
    dest_rect.x = static_cast<int>(transform.getPosition().x - dest_rect.w / 2);
    dest_rect.y = static_cast<int>(transform.getPosition().y - dest_rect.h / 2);
    //SDL_RenderCopy(r, tex, NULL, &dest_rect);
    SDL_SetRenderDrawColor(r->get_SDLRenderer(), draw_colour[0], draw_colour[1], draw_colour[2], draw_colour[3]);
    SDL_RenderDrawRect(r->get_SDLRenderer(), &dest_rect);
    return;
}


