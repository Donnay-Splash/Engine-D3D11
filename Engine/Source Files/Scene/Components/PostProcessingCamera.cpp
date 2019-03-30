#include "pch.h"
#include <Scene\Components\PostProcessingCamera.h>
#include <MeshMaker\MeshMaker.h>

namespace Engine
{
    PostProcessingCamera::PostProcessingCamera(Component::SceneNodePtr node) : Camera(node)
    {

    }

    void PostProcessingCamera::Initialize()
    {
        // initialise base camera
        Camera::Initialize();

        // Create screen aligned quad.
        m_screenAlignedQuad = Utils::MeshMaker::CreateFullScreenQuad();

        // Calculate orthogonal projection
        SetProjectionMode(Camera::ProjectionMode::Orthographic);
        SetOrthographicSize(1.0f);
    }
}