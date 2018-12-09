#pragma once
#include <Resources\Mesh.h>

namespace Utils
{
    class MeshMaker
    {
    public:
        static Engine::Mesh::Ptr CreateTriangle();

        static Engine::Mesh::Ptr CreateSphere();

        static Engine::Mesh::Ptr CreateCube();

        static Engine::Mesh::Ptr CreateTorus(float internalRadius, float externalRadius, uint32_t triDensity);

        static Engine::Mesh::Ptr CreateGroundPlane();

        static Engine::Mesh::Ptr CreateTesselatedGroundPlane(uint32_t width, uint32_t height);

        static Engine::Mesh::Ptr CreateFullScreenQuad();

    };
} // End namespace Utils