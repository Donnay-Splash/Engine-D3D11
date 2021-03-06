#include "pch.h"
#include <Scene\Components\BoundingBox.h>

namespace Engine
{
    BoundingBox::BoundingBox(SceneNodePtr sceneNode) : Component(sceneNode, L"Bounding Box")
    {

    }

    void BoundingBox::Initialize(ID3D11Device* device)
    {

    }

    void BoundingBox::Update(float frameTime)
    {

    }

    void BoundingBox::Render(ID3D11DeviceContext* deviceContext) const
    {

    }

    void BoundingBox::CalculateBoundsFromChildNodes()
    {
        auto sceneNode = GetSceneNode();

    }
}