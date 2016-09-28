#include "pch.h"
#include "InputManager.h"


InputManager::InputManager()
{
}


InputManager::~InputManager()
{
}


void InputManager::Update(InputState newInputState)
{
    m_previousState = m_currentState;
    m_currentState = newInputState;
    ProcessInput();
}


void InputManager::ProcessInput()
{
    // Reset the keys pressed this frame.
    m_keysPressedThisFrame.clear();

    // We presume that any keys pressed this frame wont aleady be in the keys stored for the input manager
    for (uint32_t keyCode : m_currentState.KeysPressed)
    {
        auto ret = m_activatedKeys.insert(keyCode);
        if (ret.second)
        {
            m_keysPressedThisFrame.insert(keyCode);
        }
    }

    for (uint32_t keyCode : m_currentState.KeysReleased)
    {
        m_activatedKeys.erase(keyCode);
    }

    m_mouseDelta = { m_currentState.PointerPosition - m_previousState.PointerPosition };
}

bool InputManager::IsKeyDown(uint32_t keyCode)
{
    return m_activatedKeys.find(keyCode) != m_activatedKeys.end();
}

bool InputManager::IsKeyPressed(uint32_t keyCode)
{
    return m_keysPressedThisFrame.find(keyCode) != m_keysPressedThisFrame.end();
}

bool InputManager::IsKeyReleased(uint32_t keyCode)
{
    return m_currentState.KeysReleased.find(keyCode) != m_currentState.KeysReleased.end();
}

bool InputManager::IsMouseButtonDown(uint32_t mouseButton)
{
    return (m_currentState.MouseButtonPressed & mouseButton) != 0;
}