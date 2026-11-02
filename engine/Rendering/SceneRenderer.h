#pragma once

#include "Core/Mesh.h"
#include "Core/Scene.h"
#include "Core/Types.h"
#include "Graphics/Buffer.h"
#include "Graphics/GraphicsTypes.h"
#include "Rendering/FrameConstants.h"
#include "ShaderLibrary.h"

#include <map>
#include <memory>
#include <utility>
#include <vector>

namespace Engine {
namespace Graphics {
class BindingLayout;
class BindingSet;
class BufferView;
class CommandContext;
class Device;
class GraphicsPipeline;
class RenderPassEncoder;
class Submission;
} // namespace Graphics

// Iterates every part of every instance in a Scene and records one draw per part.
// Usage per frame: CollectPendingUploads, Prepare (before the render pass begins),
// then Record inside the pass.
class SceneRenderer
{
public:
    ~SceneRenderer();

    bool Initialize(
        std::shared_ptr<Graphics::Device> device,
        GraphicsShaderPair shaders,
        Graphics::TextureFormat colorFormat,
        Error &error);
    void Shutdown() noexcept;

    bool CollectPendingUploads(
        const Scene &scene,
        std::vector<std::shared_ptr<Graphics::Submission>> &waitFor,
        Error &error);
    bool Prepare(const Scene &scene, Graphics::CommandContext &context, Error &error);
    bool Record(Graphics::RenderPassEncoder &renderPass, Error &error);
    // Call once the frame's command buffer has been submitted.
    void MarkSubmitted() noexcept;

    std::size_t DrawCount() const noexcept { return m_DrawItems.size(); }

private:
    struct DrawSlot
    {
        std::shared_ptr<Graphics::Buffer> buffer;
        std::shared_ptr<Graphics::BufferView> bufferView;
        std::shared_ptr<Graphics::BindingSet> bindings;
        MeshTransforms contents;
        bool written = false;
        bool prepared = false; // the one-time uniform barrier has been submitted
    };

    struct DrawItem
    {
        std::shared_ptr<Mesh> mesh;
        std::shared_ptr<Graphics::GraphicsPipeline> pipeline;
        std::size_t slot = 0;
    };

    using PipelineKey = std::pair<Graphics::CullMode, Graphics::PrimitiveTopology>;

    bool CreateSlot(Error &error);
    bool WriteSlot(DrawSlot &slot, const MeshTransforms &transforms, Error &error);
    std::shared_ptr<Graphics::GraphicsPipeline> GetPipeline(
        const PipelineKey &key, Error &error);

    std::shared_ptr<Graphics::Device> m_Device;
    GraphicsShaderPair m_Shaders;
    Graphics::TextureFormat m_ColorFormat{};
    std::shared_ptr<Graphics::BindingLayout> m_BindingLayout;
    std::map<PipelineKey, std::shared_ptr<Graphics::GraphicsPipeline>> m_Pipelines;
    std::vector<DrawSlot> m_Slots;
    std::vector<DrawItem> m_DrawItems;
};

} // namespace Engine
