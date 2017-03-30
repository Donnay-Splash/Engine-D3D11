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

    void Light::SetColor(Utils::Maths::Color color)
    {
        m_lightData.Color = color;
    }

    void Light::SetIntensity(float intensity)
    {
        m_lightIntensity = intensity;
    }

    float Light::GetIntensity() const
    {
        return m_lightIntensity;
    }

    void Light::SetShadowCastingEnabled(bool enabled)
    {
        m_castShadows = enabled;
    }

    bool Light::CastsShadows() const
    {
        return m_castShadows;
    }

    LightData Light::GetLightData()
    {
        m_lightData.Position = GetSceneNode()->GetWorldSpacePosition();
        m_lightData.Direction = GetSceneNode()->Forward();
        return m_lightData;
    }

    void Light::AddPublicProperties()
    {
        auto intensityGetter = [this]() {return m_lightIntensity; };
        auto intensitySetter = [this](float value) { m_lightIntensity = value; };
        RegisterScalarProperty(L"Intensity", intensityGetter, intensitySetter, 0.0f, 10.0f);

        auto shadowCastingGetter = [this]() {return m_castShadows; };
        auto shadowCastingSetter = [this](bool value) { m_castShadows = value; };
        RegisterBooleanProperty(L"Casts Shadows", shadowCastingGetter, shadowCastingSetter);
    }
}
