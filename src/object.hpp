#ifndef OBJECT_H
#define OBJECT_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <SDL.h>
#include <SDL_image.h>

#include "component.hpp"
#include "glm/glm.hpp"
#include "render.hpp"
#include "resource_manager.hpp"
#include "transform.hpp"

class Object
{
private:
    std::string name;
    std::vector<Object*> child_Objects;
    std::map<std::string, Component*> components;
    SDL_Rect entity_box = {-1, -1, 2, 2};
    Renderer* renderer = nullptr;

public:
    Transform2D transform;
    glm::vec4 draw_colour = glm::vec4(255.0f, 0.0f, 0.0f, 255.0f);

    Object(const std::string& obj_name, Resource_manager& resourceManager, Renderer* rend)
        : name(obj_name), renderer(rend)
    {
        (void)resourceManager;
    }

    virtual ~Object();

    void update(std::uint64_t deltaTime);
    virtual void Update(std::uint64_t deltaTime) = 0;

    void draw(Renderer* r);

    Component* add_Component(Component* component);
    int remove_Component(Component* component);
    int remove_Component(const std::string& componentName);

    int add_Child_Object(Object* object);
    int remove_Child_Object(Object* object);

    const std::string& getName() const { return name; }
};

#endif // OBJECT_H
