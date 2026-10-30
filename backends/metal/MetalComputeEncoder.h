#pragma once

#include "MetalBackendCommon.h"

namespace Engine {
namespace Metal {

class MetalComputeEncoder final : public ComputeEncoder
{
public:
    MetalComputeEncoder(std::shared_ptr<MetalCommandContext> context,
                        MTL::ComputeCommandEncoder *encoder);

    ~MetalComputeEncoder() override;

    bool End(Error &error) override;

    bool SetPipeline(const std::shared_ptr<ComputePipeline> &pipeline, Error &error) override;

    bool SetBindingSet(std::uint32_t index, const std::shared_ptr<BindingSet> &bindings,
                       Error &error) override;

    void Dispatch(std::uint32_t groupCountX, std::uint32_t groupCountY,
                  std::uint32_t groupCountZ) override;

private:
    std::shared_ptr<MetalCommandContext> context_;
    NS::SharedPtr<MTL::ComputeCommandEncoder> encoder_;
    std::shared_ptr<MetalComputePipeline> pipeline_;
    bool ended_ = false;
};

} // namespace Metal
} // namespace Engine
