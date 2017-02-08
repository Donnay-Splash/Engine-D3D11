#include "pch.h"
#include <Scene\Components\PostProcessingCamera.h>
#include <MeshMaker\MeshMaker.h>

namespace Engine
{
    PostProcessingCamera::PostProcessingCamera(Component::SceneNodePtr node) : Camera(node)
    {

    }

    void PostProcessingCamera::Initialize(ID3D11Device* device)
    {
        // initialise base camera
        Camera::Initialize(device);

        // Create screen aligned quad.
        m_screenAlignedQuad = Utils::MeshMaker::CreateFullScreenQuad(device);

        // Calculate orthogonal projection
        SetProjectionMode(Camera::ProjectionMode::Orthographic);
        SetOrthographicSize(1.0f);
    }
}