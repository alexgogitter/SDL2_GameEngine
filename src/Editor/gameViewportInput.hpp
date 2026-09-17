#pragma once

#include <glm.hpp>

struct GameViewportInputState
{
    bool keyboardEnabled = false;
    bool mouseEnabled = false;

    glm::vec2 mousePosition = {0.0f, 0.0f};
};