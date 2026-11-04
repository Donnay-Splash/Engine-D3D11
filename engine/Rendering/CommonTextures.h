#pragma once

#include "Core/Types.h"

#include <memory>

namespace Engine {
namespace Graphics {
class Device;
class TextureView;
} // namespace Graphics

// Small solid-color textures shared by every material. White is what unset material
// textures fall back to, so a descriptor is never left unbound.
class CommonTextures
{
public:
    bool Initialize(Graphics::Device &device, Error &error);
    void Shutdown() noexcept;

    const std::shared_ptr<Graphics::TextureView> &White() const noexcept { return m_White; }
    const std::shared_ptr<Graphics::TextureView> &Black() const noexcept { return m_Black; }
    // Tangent-space normal pointing straight out of the surface.
    const std::shared_ptr<Graphics::TextureView> &FlatNormal() const noexcept { return m_FlatNormal; }

private:
    std::shared_ptr<Graphics::TextureView> m_White;
    std::shared_ptr<Graphics::TextureView> m_Black;
    std::shared_ptr<Graphics::TextureView> m_FlatNormal;
};

} // namespace Engine
