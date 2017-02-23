#include "pch.h"
#include <Scene\Components\Light.h>
#include <Scene\SceneNode.h>

namespace Engine
{
    Light::Light(Component::SceneNodePtr sceneNode) : Component(sceneNode, L"Light")
    {
        AddPublicProperties();
    }

    void Light::Initialize(ID3D11Device* device)
    {

    }

    void Light::Update(float frameTime)
    {

    }

    void Light::Render(ID3D11DeviceContext* deviceContext) const
    {

    }

    void Light::SetColor(Utils::Maths::Color color)
    {
        m_lightData.Color = color;
    }


    void Light::SetDirection(Utils::Maths::Vector3 direction)
    {
        m_lightData.Direction = direction;
    }

    LightData Light::GetLightData()
    {
        m_lightData.Position = GetSceneNode()->GetWorldSpacePosition();
        return m_lightData;
    }

    void Light::AddPublicProperties()
    {
        /// TODO: Add helpers to try and cut this down to 1 line in most cases
        auto positionGetter = [this]() { return m_lightData.Direction; };
        auto positionSetter = [this](Utils::Maths::Vector3 v) { SetDirection(v); };
        RegisterVectorProperty(L"Direction", positionGetter, positionSetter);
    }
}
