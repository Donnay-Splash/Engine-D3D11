#pragma once
#include "Component.h"
#include <Utils\Math\Math.h>

class BoundingBox : public Component
{
public:
    using Ptr = std::shared_ptr<BoundingBox>;

    virtual void Update(float frameTime);
    virtual void Render(ID3D11DeviceContext* deviceContext) const;

    void SetBounds(Utils::Maths::BoundingBox newBounds) { m_aabb = newBounds; }
    Utils::Maths::BoundingBox GetBounds() const { return m_aabb; }
    void CalculateBoundsFromChildNodes();

protected:
    BoundingBox(SceneNodePtr sceneNode);
    BoundingBox(const Component&) = delete;
    virtual void Initialize(ID3D11Device* device);

private:
    Utils::Maths::BoundingBox m_aabb;
    friend class SceneNode;
};