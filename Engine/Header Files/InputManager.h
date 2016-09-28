#pragma once
#include "KeyCodes.h"
#include <set>
#include <Utils\Math\Math.h>
namespace MouseButtons
{
    static const uint32_t Left = 0x0001;
    static const uint32_t Middle = 0x0002;
    static const uint32_t Right = 0x0004;
}

struct InputState
{
    Utils::Maths::Vector2 PointerPosition;
    uint32_t MouseButtonPressed;
    std::set<uint32_t> KeysPressed;
    std::set<uint32_t> KeysReleased;
};

class InputManager
{
public:
    using Ptr = std::shared_ptr<InputManager>;
    InputManager();
    InputManager(const InputManager&) = delete;
    ~InputManager();

    void Update(InputState newInputState);

    // Returns the normalize mouse location in the range [0, 1]
    Utils::Maths::Vector2 GetMouseLocation() { return m_currentState.PointerPosition; };

    bool IsKeyDown(uint32_t keyCode);
    bool IsKeyPressed(uint32_t keyCode);
    bool IsKeyReleased(uint32_t keyCode);
    bool IsMouseButtonDown(uint32_t mouseButton);

private:
    void ProcessInput();

private:
    InputState m_currentState;
    InputState m_previousState;
    Utils::Maths::Vector2 m_mouseDelta;

    std::set<uint32_t> m_activatedKeys;
    std::set<uint32_t> m_keysPressedThisFrame;
};

