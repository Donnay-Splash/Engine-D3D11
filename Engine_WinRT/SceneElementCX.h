#pragma once
#include "PropertyCX.h"
#include <Engine\Header Files\Scene\Public Properties\SceneElement.h>
#include<collection.h>
namespace Engine_WinRT
{
    public ref class SceneElementCX sealed
    {
    public:
        property Platform::String^ Name
        {
            Platform::String^ get() 
            {
                return m_name;
            }
        }

        property bool Active
        {
            bool get()
            {
                return m_nativeSceneElement.lock() != nullptr;
            }
        }

        property Platform::Collections::Vector<PropertyCX^>^ Properties
        {
            Platform::Collections::Vector<PropertyCX^>^ get()
            {
                return m_properties;
            }
        }

    internal:
        SceneElementCX(std::weak_ptr<SceneElement> sceneElement);

    private:
        Platform::Collections::Vector<PropertyCX^>^ m_properties;
        std::weak_ptr<SceneElement> m_nativeSceneElement;
        Platform::String^ m_name;
    };

}// end namespace Engine_WinRT