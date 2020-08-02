#include "pch.h"
#include "InputManager.h"
#include "imgui.h"

namespace Engine
{
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

        // We presume that any keys pressed this frame wont already be in the keys stored for the input manager
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

        // Here we can update ImGui input
        ImGuiIO& io = ImGui::GetIO();
        io.KeyCtrl = IsKeyDown(VK_CONTROL);
        io.KeyAlt = IsKeyDown(VK_MENU);
        io.KeyShift = IsKeyDown(VK_SHIFT);
        io.KeySuper = false;
        // TODO: Find a way to get ImGui to use out Math::Vector types
        io.MousePos = ImVec2(m_currentState.PointerPosition.x, m_currentState.PointerPosition.y);
        ImGuiMouseCursor cursor = io.MouseDrawCursor ? ImGuiMouseCursor_None : ImGui::GetMouseCursor();

        // This isn't very platform agnostic. But at this point I feel like we're killing all other platforms and instead opting for a single Win32 platform
        // Maybe Console if we can be bothered
        if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) == 0)
        {
            if (cursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
            {
                ::SetCursor(NULL);
            }
            else
            {
                LPTSTR win32Cursor = IDC_ARROW;
                switch (cursor)
                {
                case ImGuiMouseCursor_Arrow:        win32Cursor = IDC_ARROW; break;
                case ImGuiMouseCursor_TextInput:    win32Cursor = IDC_IBEAM; break;
                case ImGuiMouseCursor_ResizeAll:    win32Cursor = IDC_SIZEALL; break;
                case ImGuiMouseCursor_ResizeEW:     win32Cursor = IDC_SIZEWE; break;
                case ImGuiMouseCursor_ResizeNS:     win32Cursor = IDC_SIZENS; break;
                case ImGuiMouseCursor_ResizeNESW:   win32Cursor = IDC_SIZENESW; break;
                case ImGuiMouseCursor_ResizeNWSE:   win32Cursor = IDC_SIZENWSE; break;
                case ImGuiMouseCursor_Hand:         win32Cursor = IDC_HAND; break;
                case ImGuiMouseCursor_NotAllowed:   win32Cursor = IDC_NO; break;
                }
                ::SetCursor(::LoadCursor(NULL, win32Cursor));
            }
        }
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
}