#pragma once

#include "MetalBackendCommon.h"

namespace Engine {
namespace Metal {

class MetalShader final : public Shader
{
public:
    MetalShader(ShaderStage stage, std::string entryPoint, MTL::Library *library,
                MTL::Function *function);

    ShaderStage Stage() const noexcept override;

    const std::string &EntryPoint() const noexcept override;

    MTL::Function *Native() const noexcept;

private:
    ShaderStage stage_;
    std::string entryPoint_;
    NS::SharedPtr<MTL::Library> library_;
    NS::SharedPtr<MTL::Function> function_;
};

} // namespace Metal
} // namespace Engine
