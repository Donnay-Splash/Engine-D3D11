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

        property uint32_t ID
        {
            uint32_t get()
            {
                return m_nativeSceneElement.lock()->GetID();
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

        property uint32_t ParentID
        {
            uint32_t get() { return m_parentID; }
        }

        property ElementType Type
        {
            ElementType get() { return m_type; }
        }

    internal:
        SceneElementCX(std::weak_ptr<Engine::SceneElement> sceneElement, uint32_t parentID, ElementType type);

    private:
        Platform::Collections::Vector<PropertyCX^>^ m_properties;
        std::weak_ptr<Engine::SceneElement> m_nativeSceneElement;
        Platform::String^ m_name;
        uint32_t m_parentID;

        ElementType m_type;
        
    };

}// end namespace Engine_WinRT