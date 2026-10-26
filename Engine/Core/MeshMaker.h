#pragma once

namespace Engine {

class Mesh;
struct Error;

namespace Graphics {
class Device;
}

class MeshMaker
{
public:
    static bool CreateUniformTriangle(Graphics::Device &device, Mesh &mesh, Error &error);
};

} // namespace Engine
