#pragma once

#include "engineApi.hpp"

class ComponentRegistry;

/// Registers all built-in engine component factories.
/// Safe to call more than once on the same registry.
ENGINE_API bool RegisterBuiltInComponents(
    ComponentRegistry& registry
);