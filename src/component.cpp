#include "component.hpp"

Component::Component(
    const std::string& componentTypeName,
    Object* parentObject)
    : typeName(componentTypeName),
      parent(parentObject)
{
}

Component::~Component() = default;

const std::string& Component::getTypeName() const
{
    return typeName;
}

void Component::setEnabled(bool value)
{
    if (enabled == value)
    {
        return;
    }

    enabled = value;

    if (!created)
    {
        return;
    }

    if (enabled)
    {
        OnEnable();
    }
    else
    {
        OnDisable();
    }
}

bool Component::isEnabled() const
{
    return enabled;
}

bool Component::isCreated() const
{
    return created;
}

Object* Component::getParent() const
{
    return parent;
}

void Component::create()
{
    if (created)
    {
        return;
    }

    created = true;
    OnCreate();

    if (enabled)
    {
        OnEnable();
    }
}

void Component::preUpdate(std::uint64_t deltaTime)
{
    if (created && enabled)
    {
        PreUpdate(deltaTime);
    }
}

void Component::update(std::uint64_t deltaTime)
{
    if (created && enabled)
    {
        Update(deltaTime);
    }
}

void Component::postUpdate(std::uint64_t deltaTime)
{
    if (created && enabled)
    {
        PostUpdate(deltaTime);
    }
}

void Component::destroy()
{
    if (!created)
    {
        return;
    }

    if (enabled)
    {
        OnDisable();
    }

    OnDestroy();
    created = false;
}

void Component::draw2D(Renderer2D* renderer)
{
    if (created && enabled)
    {
        Draw2D(renderer);
    }
}

void Component::draw3D(Renderer3D* renderer)
{
    if (created && enabled)
    {
        Draw3D(renderer);
    }
}

std::size_t Component::getPropertyCount() const
{
    return GetPropertyCount();
}

bool Component::getProperty(
    std::size_t index,
    ComponentProperty& property)
{
    property = {};

    if (index >= GetPropertyCount())
    {
        return false;
    }

    if (!GetProperty(index, property))
    {
        property = {};
        return false;
    }

    return
        property.key != nullptr &&
        property.key[0] != '\0' &&
        property.displayName != nullptr &&
        property.value != nullptr;
}

void Component::notifyPropertyChanged(const char* key)
{
    if (key == nullptr || key[0] == '\0')
    {
        return;
    }

    OnPropertyChanged(key);
}

void Component::notifyTransformChanged()
{
    if (created)
    {
        OnTransformChanged();
    }
}