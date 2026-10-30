#include "MetalBackendInternal.h"

namespace Engine {
namespace Metal {

using namespace Graphics;

MetalShader::MetalShader(ShaderStage stage, std::string entryPoint, MTL::Library *library,
                         MTL::Function *function)
    : stage_(stage), entryPoint_(std::move(entryPoint)), library_(NS::RetainPtr(library)),
      function_(NS::RetainPtr(function))
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

} // namespace Metal
} // namespace Engine
