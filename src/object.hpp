#ifndef OBJECT_H
#define OBJECT_H
#include <cstdint>
#include <stdio.h>
#include <SDL.h>
#include <SDL_image.h>
#include <vector>
#include "resource_manager.hpp"
#include "component.hpp"
#include "colliderComponent.hpp"
#include "glm/glm.hpp"
#include "transform.hpp"
#include "render.hpp"


class Object
{
    
private:
    std::string name;
    std::vector<Object*> child_Objects;
    std::map<std::string, Component*> components;
    SDL_Rect entity_box = {-1, -1, 2, 2};
    Renderer* renderer;

public:
    Transform2D transform;
    glm::vec4 draw_colour = glm::vec4(255.0f, 0.0f, 0.0f, 255.0f);

    Object(const std::string& obj_name, Resource_manager& r, Renderer* rend) : name(obj_name), renderer(rend) {}
    
    ~Object() {
        for (const auto& pair : components)
        {
            delete pair.second;
        }
        for (Object* child : child_Objects)
        {
            delete child;
        }
    }

    void update(std::uint64_t deltaTime);

    virtual void Update(std::uint64_t deltaTime) = 0;

    void draw(Renderer* r);

    int add_Component(Component* c) {components[c->name] = c; return 0;};

    int remove_Component(Component* c) {components.erase(c->name); return 0;};

    int remove_Component(const std::string& name) {components.erase(name); return 0;};

    int add_Child_Object(Object* o) {child_Objects.push_back(o); return 0;};

    int remove_Child_Object(Object* o) 
    {
        for (Object* child : child_Objects)
        {
            if (child == o)
            {
                delete child;
                return 0;
            }
        }
        return -1; // Object not found
    };

};

#endif // OBJECT_H