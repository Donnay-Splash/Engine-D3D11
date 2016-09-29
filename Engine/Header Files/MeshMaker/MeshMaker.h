#pragma once
#include <Resources\Mesh.h>

namespace Utils
{
    class MeshMaker
    {
    public:
        static Mesh::Ptr CreateTriangle(ID3D11Device* device);

        static Mesh::Ptr CreateSphere(ID3D11Device* device);

        static Mesh::Ptr CreateCube(ID3D11Device* device);

        static Mesh::Ptr CreateGroundPlane(ID3D11Device* device);

        static Mesh::Ptr CreateTesselatedGroundPlane(ID3D11Device* device, uint32_t width, uint32_t height);

        static Mesh::Ptr CreateFullScreenQuad(ID3D11Device* device);
    };
} // End namespace Utils