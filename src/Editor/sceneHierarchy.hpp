#pragma once

#include "engineApi.hpp"

class EditorSelection;
class Scene;

/// Draws the scene hierarchy and returns true when it changes.
ENGINE_API bool DrawSceneHierarchy(Scene &scene, EditorSelection &selection);