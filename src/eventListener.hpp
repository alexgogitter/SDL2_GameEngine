#pragma once

#include <SDL.h>

#include <array>
#include <cstddef>
#include <cstdint>

class EventListener
{
public:
    struct ButtonState
    {
        bool down = false;
        bool pressedThisFrame = false;
        bool releasedThisFrame = false;

        Uint64 pressedAtMs = 0;
        Uint64 releasedAtMs = 0;
    };

    struct MouseState
    {
        int x = 0;
        int y = 0;

        // Movement during the current frame.
        int deltaX = 0;
        int deltaY = 0;

        // Movement accumulated since startup or ResetMouseTotals().
        Sint64 totalDeltaX = 0;
        Sint64 totalDeltaY = 0;

        Uint64 movementEventCount = 0;
        bool movedThisFrame = false;
    };

    static EventListener& Get();

    EventListener(const EventListener&) = delete;
    EventListener& operator=(const EventListener&) = delete;

    // Call once at the start of every game frame.
    void BeginFrame();

    // Pass every SDL event through this method.
    void ProcessEvent(const SDL_Event& event);

    // Keyboard queries.
    const ButtonState& GetKeyState(SDL_Scancode key) const;
    bool IsKeyDown(SDL_Scancode key) const;
    bool WasKeyPressed(SDL_Scancode key) const;
    bool WasKeyReleased(SDL_Scancode key) const;
    Uint64 GetKeyHeldTimeMs(SDL_Scancode key) const;

    // Mouse button queries.
    const ButtonState& GetMouseButtonState(Uint8 button) const;
    bool IsMouseButtonDown(Uint8 button) const;
    bool WasMouseButtonPressed(Uint8 button) const;
    bool WasMouseButtonReleased(Uint8 button) const;
    Uint64 GetMouseButtonHeldTimeMs(Uint8 button) const;

    // Mouse position and movement.
    const MouseState& GetMouseState() const;
    void ResetMouseTotals();

    // Clears button states, useful after focus is lost.
    void ResetAll();

private:
    EventListener() = default;

    static constexpr std::size_t MouseButtonCount = SDL_BUTTON_X2 + 1;

    std::array<ButtonState, SDL_NUM_SCANCODES> keyStates_{};
    std::array<ButtonState, MouseButtonCount> mouseButtonStates_{};

    MouseState mouseState_{};
    ButtonState invalidButtonState_{};

    void PressButton(ButtonState& state, Uint64 nowMs);
    void ReleaseButton(ButtonState& state, Uint64 nowMs);

    bool IsValidKey(SDL_Scancode key) const;
    bool IsValidMouseButton(Uint8 button) const;
};