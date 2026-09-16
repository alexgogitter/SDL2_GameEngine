#pragma once

#include "engineApi.hpp"

class Component;

/// Draws enabled state and all reflected component properties.
/// Returns true when anything changed.
ENGINE_API bool DrawComponentInspector(Component& component);