#pragma once

#include "MetalBackendCommon.h"

namespace Engine {
namespace Metal {

struct MetalBindingLocation
{
    std::uint32_t slot = 0;
    std::uint32_t space = 0;
    std::size_t offset = 0;
    std::size_t size = 0;
    std::string type;
};

class MetalShader final : public Shader
{
public:
    MetalShader(ShaderStage stage, std::string entryPoint, MTL::Library *library,
                MTL::Function *function, std::vector<MetalBindingLocation> bindingLocations);

    ShaderStage Stage() const noexcept override;

    const std::string &EntryPoint() const noexcept override;

    MTL::Function *Native() const noexcept;

    const std::vector<MetalBindingLocation> &BindingLocations() const noexcept;

private:
    ShaderStage stage_;
    std::string entryPoint_;
    NS::SharedPtr<MTL::Library> library_;
    NS::SharedPtr<MTL::Function> function_;
    std::vector<MetalBindingLocation> bindingLocations_;
};

} // namespace Metal
} // namespace Engine
