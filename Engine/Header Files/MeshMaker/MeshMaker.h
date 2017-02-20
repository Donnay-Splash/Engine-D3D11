#pragma once
#include <Resources\Mesh.h>

namespace Utils
{
    class MeshMaker
    {
    public:
        static Engine::Mesh::Ptr CreateTriangle(ID3D11Device* device);

        static Engine::Mesh::Ptr CreateSphere(ID3D11Device* device);

        static Engine::Mesh::Ptr CreateCube(ID3D11Device* device);

        static Engine::Mesh::Ptr CreateTorus(ID3D11Device* device, float internalRadius, float externalRadius, uint32_t triDensity);

        static Engine::Mesh::Ptr CreateGroundPlane(ID3D11Device* device);

        static Engine::Mesh::Ptr CreateTesselatedGroundPlane(ID3D11Device* device, uint32_t width, uint32_t height);

        static Engine::Mesh::Ptr CreateFullScreenQuad(ID3D11Device* device);

    };
} // End namespace Utils