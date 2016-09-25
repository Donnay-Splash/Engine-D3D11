#include "pch.h"
#include "InputManager.h"


InputManager::InputManager()
{
}


InputManager::~InputManager()
{
}


void InputManager::Shutdown()
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
    // We presume that any keys pressed this frame wont aleady be in the keys stored for the input manager
    for (uint32_t keyCode : m_currentState.KeysPressed)
    {
        m_pressedKeys.insert(keyCode);
    }

    for (uint32_t keyCode : m_currentState.KeysReleased)
    {
        m_pressedKeys.erase(keyCode);
    }

    m_mouseDelta = { m_currentState.PointerPosition - m_previousState.PointerPosition };
}

bool InputManager::IsKeyDown(uint32_t keyCode)
{
    return m_pressedKeys.find(keyCode) != m_pressedKeys.end();
}

bool InputManager::IsKeyPressed(uint32_t keyCode)
{
    return m_currentState.KeysPressed.find(keyCode) != m_currentState.KeysPressed.end();
}

bool InputManager::IsKeyReleased(uint32_t keyCode)
{
    return m_currentState.KeysReleased.find(keyCode) != m_currentState.KeysReleased.end();
}

bool InputManager::IsMouseButtonDown(uint32_t mouseButton)
{
    return (m_currentState.MouseButtonPressed & mouseButton) != 0;
}