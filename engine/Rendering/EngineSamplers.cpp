#include "Rendering/EngineSamplers.h"

#include "Graphics/Device.h"
#include "Graphics/Sampler.h"

namespace Engine {

bool EngineSamplers::Initialize(Graphics::Device &device, Error &error)
{
    struct Definition
    {
        const char *name;
        Graphics::FilterMode filter;
        Graphics::AddressMode address;
    };
    // The registers of these names in HLSL are s0, s1 and s2 respectively.
    static const Definition definitions[] = {
        {"linearRepeat", Graphics::FilterMode::Linear, Graphics::AddressMode::Repeat},
        {"linearClamp", Graphics::FilterMode::Linear, Graphics::AddressMode::ClampToEdge},
        {"pointClamp", Graphics::FilterMode::Nearest, Graphics::AddressMode::ClampToEdge},
    };
    m_Samplers.clear();
    for (const Definition &definition : definitions) {
        Graphics::SamplerDesc description;
        description.minFilter = definition.filter;
        description.magFilter = definition.filter;
        description.addressU = definition.address;
        description.addressV = definition.address;
        description.addressW = definition.address;
        std::shared_ptr<Graphics::Sampler> sampler = device.CreateSampler(description, error);
        if (sampler == nullptr) {
            if (error.message.empty()) {
                error.message = std::string("Failed to create the engine sampler '")
                    + definition.name + "'.";
            }
            m_Samplers.clear();
            return false;
        }
        m_Samplers.push_back({definition.name, std::move(sampler)});
    }
    return true;
}

void EngineSamplers::Shutdown() noexcept
{
    m_Samplers.clear();
}

std::shared_ptr<Graphics::Sampler> EngineSamplers::Find(const std::string &name) const
{
    for (const Entry &entry : m_Samplers) {
        if (entry.name == name) {
            return entry.sampler;
        }
    }
    return nullptr;
}

} // namespace Engine
