#include "object.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>

Object::~Object()
{
    for (const auto& pair : components)
    {
        delete pair.second;
    }

    for (Object* child : child_Objects)
    {
        delete child;
    }
}

void Object::update(std::uint64_t deltaTime)
{
    Update(deltaTime);

    for (const auto& pair : components)
    {
        pair.second->update(deltaTime);
    }

    for (Object* child : child_Objects)
    {
        child->update(deltaTime);
    }
}

void Object::draw(Renderer* r)
{
    if (r == nullptr)
    {
        std::fprintf(stderr, "ERROR: Renderer is null in Object::draw().\n");
        return;
    }

    const float halfWidth = transform.getScale().x;
    const float halfHeight = transform.getScale().y;

    const glm::vec2 centre = transform.getPosition();
    const float angle = transform.getRotation();

    const float cosine = std::cos(angle);
    const float sine = std::sin(angle);

    auto rotatePoint = [&](float localX, float localY) -> SDL_FPoint
    {
        return {
            centre.x + (localX * cosine - localY * sine),
            centre.y + (localX * sine + localY * cosine)
        };
    };

    const SDL_FPoint topLeft     = rotatePoint(-halfWidth, -halfHeight);
    const SDL_FPoint topRight    = rotatePoint( halfWidth, -halfHeight);
    const SDL_FPoint bottomRight = rotatePoint( halfWidth,  halfHeight);
    const SDL_FPoint bottomLeft  = rotatePoint(-halfWidth,  halfHeight);

    const SDL_Color fillColour = {
        static_cast<Uint8>(draw_colour.r),
        static_cast<Uint8>(draw_colour.g),
        static_cast<Uint8>(draw_colour.b),
        static_cast<Uint8>(draw_colour.a)
    };

    SDL_Vertex vertices[4] = {
        { topLeft,     fillColour, { 0.0f, 0.0f } },
        { topRight,    fillColour, { 1.0f, 0.0f } },
        { bottomRight, fillColour, { 1.0f, 1.0f } },
        { bottomLeft,  fillColour, { 0.0f, 1.0f } }
    };

    const int indices[6] = {
        0, 1, 2,
        0, 2, 3
    };

    SDL_RenderGeometry(
        r->get_SDLRenderer(),
        nullptr,
        vertices,
        4,
        indices,
        6
    );

    // Rendering is component-driven. A TextureComponent, for example, draws
    // over the colour quad while sharing this object's physics transform.
    for (const auto& pair : components)
    {
        pair.second->draw(r);
    }

    SDL_SetRenderDrawColor(r->get_SDLRenderer(), 20, 20, 24, 255);

    SDL_RenderDrawLineF(r->get_SDLRenderer(), topLeft.x, topLeft.y, topRight.x, topRight.y);
    SDL_RenderDrawLineF(r->get_SDLRenderer(), topRight.x, topRight.y, bottomRight.x, bottomRight.y);
    SDL_RenderDrawLineF(r->get_SDLRenderer(), bottomRight.x, bottomRight.y, bottomLeft.x, bottomLeft.y);
    SDL_RenderDrawLineF(r->get_SDLRenderer(), bottomLeft.x, bottomLeft.y, topLeft.x, topLeft.y);
}

Component* Object::add_Component(Component* component)
{
    if (component == nullptr)
    {
        return nullptr;
    }

    const auto existing = components.find(component->name);
    if (existing != components.end())
    {
        delete existing->second;
        existing->second = component;
        return component;
    }

    components[component->name] = component;
    return component;
}

int Object::remove_Component(Component* component)
{
    if (component == nullptr)
    {
        return -1;
    }

    return remove_Component(component->name);
}

int Object::remove_Component(const std::string& componentName)
{
    const auto found = components.find(componentName);
    if (found == components.end())
    {
        return -1;
    }

    delete found->second;
    components.erase(found);
    return 0;
}

int Object::add_Child_Object(Object* object)
{
    if (object == nullptr)
    {
        return -1;
    }

    child_Objects.push_back(object);
    return 0;
}

int Object::remove_Child_Object(Object* object)
{
    const auto found = std::find(child_Objects.begin(), child_Objects.end(), object);
    if (found == child_Objects.end())
    {
        return -1;
    }

    delete *found;
    child_Objects.erase(found);
    return 0;
}
