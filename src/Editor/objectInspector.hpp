#pragma once

#include <algorithm>
#include <cctype>
#include <functional>
#include <vector>

#include "component.hpp"
#include "engineApi.hpp"

class LayerRegistry;
class Object;
class ComponentRegistry;

struct ComponentCreateContext;

/// Editor services used exclusively by script entries in the component viewer.
struct ScriptComponentActions
{
    std::function<bool(const std::string &name, std::string &error)> createScript;
    std::function<bool(const std::string &typeName, std::string &error)> editScript;
};

/// Draws an object's core data, transform, and components.
ENGINE_API bool DrawObjectInspector(Object &object, LayerRegistry &layers, ComponentRegistry &componentRegistry,
                                    const ComponentCreateContext &componentContext, bool structuralEditingAllowed,
                                    const ScriptComponentActions *scriptActions = nullptr);
