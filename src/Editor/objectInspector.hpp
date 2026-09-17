#pragma once

#include <algorithm>
#include <cctype>
#include <vector>

#include "component.hpp"
#include "engineApi.hpp"

class LayerRegistry;
class Object;
class ComponentRegistry;

struct ComponentCreateContext;

/// Draws an object's core data, transform, and components.
ENGINE_API bool DrawObjectInspector(Object &object, LayerRegistry &layers, ComponentRegistry &componentRegistry, const ComponentCreateContext &componentContext, bool structuralEditingAllowed);