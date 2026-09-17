#include "eventListener.hpp"

EventListener &EventListener::Get()
{
    static EventListener instance;
    return instance;
}

void EventListener::BeginFrame()
{
    for (ButtonState &state : keyStates_) {
        state.pressedThisFrame = false;
        state.releasedThisFrame = false;
    }

    for (ButtonState &state : mouseButtonStates_) {
        state.pressedThisFrame = false;
        state.releasedThisFrame = false;
    }

    mouseState_.deltaX = 0;
    mouseState_.deltaY = 0;
    mouseState_.movedThisFrame = false;
}

void EventListener::ProcessEvent(const SDL_Event &event)
{
    const Uint64 nowMs = SDL_GetTicks64();

    switch (event.type) {
    case SDL_KEYDOWN: {
        const SDL_Scancode key = event.key.keysym.scancode;

        if (IsValidKey(key)) {
            PressButton(keyStates_[static_cast<std::size_t>(key)], nowMs);
        }

        break;
    }

    case SDL_KEYUP: {
        const SDL_Scancode key = event.key.keysym.scancode;

        if (IsValidKey(key)) {
            ReleaseButton(keyStates_[static_cast<std::size_t>(key)], nowMs);
        }

        break;
    }

    case SDL_MOUSEBUTTONDOWN: {
        const Uint8 button = event.button.button;

        mouseState_.x = event.button.x;
        mouseState_.y = event.button.y;

        if (IsValidMouseButton(button)) {
            PressButton(mouseButtonStates_[button], nowMs);
        }

        break;
    }

    case SDL_MOUSEBUTTONUP: {
        const Uint8 button = event.button.button;

        mouseState_.x = event.button.x;
        mouseState_.y = event.button.y;

        if (IsValidMouseButton(button)) {
            ReleaseButton(mouseButtonStates_[button], nowMs);
        }

        break;
    }

    case SDL_MOUSEMOTION: {
        mouseState_.x = event.motion.x;
        mouseState_.y = event.motion.y;

        mouseState_.deltaX += event.motion.xrel;
        mouseState_.deltaY += event.motion.yrel;

        mouseState_.totalDeltaX += event.motion.xrel;
        mouseState_.totalDeltaY += event.motion.yrel;

        mouseState_.movementEventCount++;

        if (event.motion.xrel != 0 || event.motion.yrel != 0) {
            mouseState_.movedThisFrame = true;
        }

        break;
    }

    case SDL_WINDOWEVENT: {
        if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
            ResetAll();
        }

        break;
    }

    default:
        break;
    }
}

const EventListener::ButtonState &EventListener::GetKeyState(SDL_Scancode key) const
{
    if (!IsValidKey(key)) {
        return invalidButtonState_;
    }

    return keyStates_[static_cast<std::size_t>(key)];
}

bool EventListener::IsKeyDown(SDL_Scancode key) const { return GetKeyState(key).down; }

bool EventListener::WasKeyPressed(SDL_Scancode key) const { return GetKeyState(key).pressedThisFrame; }

bool EventListener::WasKeyReleased(SDL_Scancode key) const { return GetKeyState(key).releasedThisFrame; }

Uint64 EventListener::GetKeyHeldTimeMs(SDL_Scancode key) const
{
    const ButtonState &state = GetKeyState(key);

    if (!state.down) {
        return 0;
    }

    return SDL_GetTicks64() - state.pressedAtMs;
}

const EventListener::ButtonState &EventListener::GetMouseButtonState(Uint8 button) const
{
    if (!IsValidMouseButton(button)) {
        return invalidButtonState_;
    }

    return mouseButtonStates_[button];
}

bool EventListener::IsMouseButtonDown(Uint8 button) const { return GetMouseButtonState(button).down; }

bool EventListener::WasMouseButtonPressed(Uint8 button) const { return GetMouseButtonState(button).pressedThisFrame; }

bool EventListener::WasMouseButtonReleased(Uint8 button) const { return GetMouseButtonState(button).releasedThisFrame; }

Uint64 EventListener::GetMouseButtonHeldTimeMs(Uint8 button) const
{
    const ButtonState &state = GetMouseButtonState(button);

    if (!state.down) {
        return 0;
    }

    return SDL_GetTicks64() - state.pressedAtMs;
}

const EventListener::MouseState &EventListener::GetMouseState() const { return mouseState_; }

void EventListener::ResetMouseTotals()
{
    mouseState_.totalDeltaX = 0;
    mouseState_.totalDeltaY = 0;
    mouseState_.movementEventCount = 0;
}

void EventListener::ResetAll()
{
    for (ButtonState &state : keyStates_) {
        state = ButtonState{};
    }

    for (ButtonState &state : mouseButtonStates_) {
        state = ButtonState{};
    }

    mouseState_ = MouseState{};
}

void EventListener::PressButton(ButtonState &state, Uint64 nowMs)
{
    // SDL key repeat generates more KEYDOWN events.
    // Only record the first press, not repeats.
    if (!state.down) {
        state.down = true;
        state.pressedThisFrame = true;
        state.pressedAtMs = nowMs;
    }
}

void EventListener::ReleaseButton(ButtonState &state, Uint64 nowMs)
{
    if (state.down) {
        state.down = false;
        state.releasedThisFrame = true;
        state.releasedAtMs = nowMs;
    }
}

bool EventListener::IsValidKey(SDL_Scancode key) const
{
    const int value = static_cast<int>(key);

    return value >= 0 && value < SDL_NUM_SCANCODES;
}

bool EventListener::IsValidMouseButton(Uint8 button) const { return button > 0 && button < MouseButtonCount; }