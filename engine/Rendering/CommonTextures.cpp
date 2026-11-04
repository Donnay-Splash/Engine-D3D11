#include "Rendering/CommonTextures.h"

#include "Graphics/Device.h"
#include "Graphics/TextureView.h"
#include "Rendering/TextureUpload.h"

#include <cstdint>

namespace Engine {

bool CommonTextures::Initialize(Graphics::Device &device, Error &error)
{
    const auto solid = [&](const std::uint8_t (&rgba)[4]) {
        return CreateTexture2D(device, Graphics::TextureFormat::RGBA8Unorm, 1, 1, rgba, error);
    };
    static const std::uint8_t white[4] = {255, 255, 255, 255};
    static const std::uint8_t black[4] = {0, 0, 0, 255};
    static const std::uint8_t flatNormal[4] = {128, 128, 255, 255};
    m_White = solid(white);
    m_Black = m_White == nullptr ? nullptr : solid(black);
    m_FlatNormal = m_Black == nullptr ? nullptr : solid(flatNormal);
    if (m_FlatNormal == nullptr) {
        Shutdown();
        return false;
    }
    return true;
}

void CommonTextures::Shutdown() noexcept
{
    m_White.reset();
    m_Black.reset();
    m_FlatNormal.reset();
}

} // namespace Engine
