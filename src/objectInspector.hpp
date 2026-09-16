#pragma once

#include <algorithm>
#include <cctype>
#include <vector>

#include "engineApi.hpp"
#include "component.hpp"

class LayerRegistry;
class Object;
class ComponentRegistry;

struct ComponentCreateContext;

/// Draws an object's core data, transform, and components.
ENGINE_API bool DrawObjectInspector(
    Object& object,
    LayerRegistry& layers,
    ComponentRegistry& componentRegistry,
    const ComponentCreateContext& componentContext,
    bool structuralEditingAllowed
);