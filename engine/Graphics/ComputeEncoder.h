#pragma once

#include "BindingSet.h"
#include "CommandEncoder.h"
#include "ComputePipeline.h"

#include <cstdint>
#include <memory>

namespace Engine {
struct Error;
namespace Graphics {

class ComputeEncoder : public CommandEncoder
{
public:
    virtual bool SetPipeline(
        const std::shared_ptr<ComputePipeline> &pipeline,
        ::Engine::Error &error) = 0;
    virtual bool SetBindingSet(
        std::uint32_t index,
        const std::shared_ptr<BindingSet> &bindings,
        ::Engine::Error &error) = 0;
    virtual void Dispatch(
        std::uint32_t groupCountX,
        std::uint32_t groupCountY,
        std::uint32_t groupCountZ) = 0;
};

} // namespace Graphics
} // namespace Engine
