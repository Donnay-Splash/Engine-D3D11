#pragma once
#include "PropertyCX.h"
#include <Engine\Header Files\Scene\Public Properties\SceneElement.h>
#include<collection.h>
namespace Engine_WinRT
{
    public enum class ElementType
    {
        SceneNode,
        Component,
        Resource
    };

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


        property Windows::Foundation::Collections::IVector<PropertyCX^>^ Properties
        {
            Windows::Foundation::Collections::IVector<PropertyCX^>^ get()
            {
                return m_properties;
            }
        }

        property Platform::String^ ParentName
        {
            Platform::String^ get() { return m_parentName; }
        }

        property ElementType Type
        {
            ElementType get() { return m_type; }
        }

    internal:
        SceneElementCX(std::weak_ptr<Engine::SceneElement> sceneElement, std::wstring parentName, ElementType type);

    private:
        Platform::Collections::Vector<PropertyCX^>^ m_properties;
        std::weak_ptr<Engine::SceneElement> m_nativeSceneElement;
        Platform::String^ m_name;
        Platform::String^ m_parentName;

        ElementType m_type;
        
    };

}// end namespace Engine_WinRT