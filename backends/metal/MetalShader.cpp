#include "MetalBackendInternal.h"

namespace Engine {
namespace Metal {

using namespace Graphics;

MetalShader::MetalShader(ShaderStage stage, std::string entryPoint, MTL::Library *library,
                         MTL::Function *function,
                         std::vector<MetalBindingLocation> bindingLocations)
    : stage_(stage), entryPoint_(std::move(entryPoint)), library_(NS::RetainPtr(library)),
      function_(NS::RetainPtr(function)), bindingLocations_(std::move(bindingLocations))
{
}

ShaderStage MetalShader::Stage() const noexcept
{
    return stage_;
}

const std::string &MetalShader::EntryPoint() const noexcept
{
    return entryPoint_;
}

MTL::Function *MetalShader::Native() const noexcept
{
    return function_.get();
}

const std::vector<MetalBindingLocation> &MetalShader::BindingLocations() const noexcept
{
    return bindingLocations_;
}

} // namespace Metal
} // namespace Engine
