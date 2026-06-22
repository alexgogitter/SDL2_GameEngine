#pragma once

#include <cstdint>
#include <string>

class Object;
class Renderer;

class Component
{
protected:
    Object* parent = nullptr;

    virtual void Update(std::uint64_t deltaTime) = 0;
    virtual void Draw(Renderer*) {}

public:
    std::string name;

    Component(const std::string& componentName, Object* parentObject)
        : parent(parentObject), name(componentName)
    {
    }

    virtual ~Component() = default;

    void update(std::uint64_t deltaTime);
    void draw(Renderer* renderer);

    Object* getParent() const { return parent; }
};
