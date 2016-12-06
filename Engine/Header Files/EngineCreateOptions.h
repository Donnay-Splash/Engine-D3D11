#pragma once
#include <functional>
#include <Scene\Public Properties\SceneElement.h>

namespace Engine
{
    enum class EngineRendererMode
    {
        Win32,
        XAML
    };

    struct EngineCreateOptions
    {
        EngineRendererMode RendererMode = EngineRendererMode::XAML;
        HWND HWND = NULL;
        bool FullScreen = false;
        bool VSyncEnabled = true;
        uint32_t ScreenWidth;
        uint32_t ScreenHeight;
        void* UserData = nullptr;
        std::function<void(IUnknown* swapChain, void* userData)> SwapChainCreatedCallback = nullptr;
        SceneElement::ChildElementsChangedEventHandler RootSceneElementAddedCallback = nullptr;
    };
}