#pragma once
#include "Math.h"
#include "Component.h"

namespace Engine
{
    // light information to be consumed by shader
    // TODO: May need to add padding
    struct LightData
    {
        Utils::Maths::Vector3 Position;
        Utils::Maths::Vector3 Direction;
        Utils::Maths::Color Color;

        void Transform(const Utils::Maths::Matrix& transform)
        {
            Position = Position * transform;
            Direction = transform.TransformNormal(Direction);
        }

        bool operator!=(const LightData& rhs)
        {
            return  (this->Position != rhs.Position) ||
                (this->Direction != rhs.Direction) ||
                (this->Color != rhs.Color);
        }

        bool operator==(const LightData& rhs)
        {
            return  (this->Position == rhs.Position) &&
                (this->Direction == rhs.Direction) &&
                (this->Color == rhs.Color);
        }
    };

    class Light : public Component
    {
    public:
        using Ptr = std::shared_ptr<Light>;
        void SetColor(Utils::Maths::Color);
        void SetIntensity(float intensity);

        LightData GetLightData();
        float GetIntensity() const;
        bool CastsShadows() const;
        void SetShadowCastingEnabled(bool enable);

    protected:
        Light(Component::SceneNodePtr sceneNode);
        virtual void Initialize() override;

    private:
        void AddPublicProperties();

    private:
        LightData m_lightData;
        // Doesn't use physically correct values but we don't care.
        float m_lightIntensity = 1.0f;
        bool m_castShadows = false;

        friend class SceneNode;
    };
}

