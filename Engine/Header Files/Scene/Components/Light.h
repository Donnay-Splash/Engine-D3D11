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
        virtual void Update(float frameTime) override;
        virtual void Render(ID3D11DeviceContext* deviceContext) const override;

        void SetColor(Utils::Maths::Color);
        void SetDirection(Utils::Maths::Vector3);

        LightData GetLightData();

    protected:
        Light(Component::SceneNodePtr sceneNode);
        virtual void Initialize(ID3D11Device* device) override;

    private:
        void AddPublicProperties();

    private:
        LightData m_lightData;

        friend class SceneNode;
    };
}

