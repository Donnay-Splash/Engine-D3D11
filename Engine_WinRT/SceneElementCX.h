#pragma once
#include "PropertyCX.h"
#include <Engine\Header Files\Scene\Public Properties\SceneElement.h>
#include<collection.h>
namespace Engine_WinRT
{
    public ref class SceneElementCX sealed
    {
    public:
    internal:
        SceneElementCX(std::weak_ptr<SceneElement> sceneElement);
    private:
        Platform::Collections::Vector<PropertyCX^>^ m_properties;
        std::weak_ptr<SceneElement> m_nativeSceneElement;
    };

}// end namespace Engine_WinRT