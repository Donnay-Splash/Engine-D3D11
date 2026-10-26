# Portable graphics-core abstraction proposal

**Status:** Portable interfaces and a single-mesh frontend recording path are implemented; no GPU backend implements the graphics contracts yet. This document remains the design reference.

## Executive summary

The portable engine should describe *what work is required*—resource usage, bindings, pipeline state, render attachments, and dependencies—while backend implementations translate that intent into Metal, Direct3D 12, or Vulkan operations. The three APIs share important nouns, but their synchronization, memory, resource binding, and presentation mechanics are not interchangeable. A generic interface should preserve those differences behind backend implementations and capability queries instead of pretending they do not exist.

Recommended first principles:

1. Distinguish GPU `Graphics::Device` from the former `Engine::Device`. The old lifecycle interface is no longer used by `Engine`; the legacy `Renderer` declaration and Metal implementation remain unintegrated and are not the lower-level GPU device represented by `MTLDevice`, `ID3D12Device`, or `VkDevice`.
2. Model modern command recording, queue submission, and completion separately. `CommandContext` should be a scoped recorder, not a D3D11-style persistent immediate context.
3. Use `std::shared_ptr<Resource>` as the public ownership model, as requested. Keep a resource's backend state, descriptors, and associated allocations alive until GPU completion for the resource's final use is known. A fixed one-frame delay is not a completion guarantee.
4. Separate resource storage from resource views, shader artifacts from pipeline state, and the host `Surface` from image acquisition and presentation.
5. Keep feature availability, allocation intent, and synchronization scope explicit. No single common feature set or one-to-one native handle abstraction is assumed.

This proposal fits the repository's established split: portable C++ in `engine/`, backend code in separate libraries, and a thin OS host. The current Metal backend still contains legacy device, queue, and shader setup in [MetalRenderer.mm](../backends/metal/MetalRenderer.mm); it does not implement the new contracts. `Engine` now depends on injected graphics-instance and shader-library interfaces, but the macOS factory deliberately supplies neither until a backend adopts them. The existing [macOS Metal engine design](./macos-metal-engine-tdd.md) says D3D12 and Vulkan are future evaluation targets and warns against recreating D3D11's immediate-context/resource API.

## What the APIs actually expose

| Concern | Metal | Direct3D 12 | Vulkan | Portable implication |
|---|---|---|---|---|
| **Adapter and device** | `MTLDevice` is the GPU-facing object used for capability queries and creating resources and pipeline state. See [MTLDevice](https://developer.apple.com/documentation/metal/mtldevice). | DXGI enumerates adapters; an `ID3D12Device` is created for a selected adapter. See [DXGI adapter enumeration](https://learn.microsoft.com/en-us/windows/win32/api/dxgi1_6/nf-dxgi1_6-idxgifactory6-enumadapterbygpupreference) and [D3D12CreateDevice](https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-d3d12createdevice). | An instance enumerates physical devices, then a selected physical device creates a logical device with selected features and queues. See the Vulkan Specification's [Devices and Queues](https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#devsandqueues). | Keep adapter discovery and capability selection distinct from logical-device creation. Treat optional features as queried/enabled capabilities, not universal promises. |
| **Command recording and submission** | A `MTLCommandQueue` creates command buffers; render/compute/blit encoders record work, and command buffers are committed. See [MTLCommandQueue](https://developer.apple.com/documentation/metal/mtlcommandqueue), [MTLCommandBuffer](https://developer.apple.com/documentation/metal/mtlcommandbuffer), and [MTLCommandEncoder](https://developer.apple.com/documentation/metal/mtlcommandencoder). | Applications record command lists, then submit them to command queues. D3D12 deliberately has no immediate context; recording can be distributed across threads. See Microsoft's [Work submission in Direct3D 12](https://learn.microsoft.com/en-us/windows/win32/direct3d12/command-queues-and-command-lists). | Command buffers are allocated from command pools, recorded, and submitted to queues selected from queue families. See the specification's [Devices and Queues](https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#devsandqueues) and [Command Buffers](https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#commandbuffers). | Separate recording, encoded command buffers, queues, submissions, and completion. A `CommandContext` can make recording ergonomic, but should not collapse these lifetimes into one global immediate context. |
| **Resources and allocation** | `MTLBuffer` and `MTLTexture` are resource objects with usage/storage options; `MTLHeap` supports heap allocation. See [MTLResource](https://developer.apple.com/documentation/metal/mtlresource), [MTLBuffer](https://developer.apple.com/documentation/metal/mtlbuffer), [MTLTexture](https://developer.apple.com/documentation/metal/mtltexture), [MTLHeap](https://developer.apple.com/documentation/metal/mtlheap), and [MTLResourceOptions](https://developer.apple.com/documentation/metal/mtlresourceoptions). | `ID3D12Resource` represents buffers/textures; resources may be committed individually or placed in heaps. See [D3D12 resources](https://learn.microsoft.com/en-us/windows/win32/direct3d12/resources), [committed resources](https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12device-createcommittedresource), and [placed resources](https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12device-createplacedresource). | `VkBuffer`/`VkImage` are bound to separately allocated `VkDeviceMemory`, subject to memory requirements; image views describe subresources. See specification sections [Resources](https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#resources), [Memory Allocation](https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#memory), and [Image Views](https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#resources-image-views). | Keep resource description, view/subresource range, and allocation service distinct. Express memory intent (device-local, upload, readback, transient) without promising identical residency, coherency, or placement behavior. |
| **Shaders, pipelines, and bindings** | Shader functions from a library are used to build render/compute pipeline state; resources are bound through encoder slots or argument buffers. See [MTLLibrary](https://developer.apple.com/documentation/metal/mtllibrary), [MTLRenderPipelineState](https://developer.apple.com/documentation/metal/mtlrenderpipelinestate), [MTLComputePipelineState](https://developer.apple.com/documentation/metal/mtlcomputepipelinestate), and [Argument Buffers](https://developer.apple.com/documentation/metal/argument_buffers). | PSOs capture shader bytecode and fixed-function state; root signatures describe shader bindings and descriptor heaps store descriptors. See [Graphics pipeline](https://learn.microsoft.com/en-us/windows/win32/direct3d12/graphics-pipeline), [root signatures](https://learn.microsoft.com/en-us/windows/win32/direct3d12/creating-a-root-signature), and [descriptor heaps](https://learn.microsoft.com/en-us/windows/win32/direct3d12/descriptor-heaps-overview). | Shader modules feed pipelines; pipeline layouts combine descriptor set layouts and push-constant ranges. See specification sections [Pipelines](https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#pipelines), [Pipeline Layouts](https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#pipelines-layout), and [Descriptor Sets](https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#descriptorsets). | Separate `Shader`, binding layout/set, and graphics/compute pipeline concepts. Do not prescribe one shader source language or imply that native binding models have the same cost or update rules. |
| **Synchronization and resource state** | Metal provides fences/events and encoder-level memory barriers; hazard behavior is expressed differently from explicit state-transition APIs. See [MTLFence](https://developer.apple.com/documentation/metal/mtlfence), [MTLEvent](https://developer.apple.com/documentation/metal/mtlevent), and [render encoder memory barrier](https://developer.apple.com/documentation/metal/mtlrendercommandencoder/memorybarrier(scope:after:)). | Resource transitions, UAV/aliasing barriers, and fences are explicit. See [Using Resource Barriers](https://learn.microsoft.com/en-us/windows/win32/direct3d12/using-resource-barriers-to-synchronize-resource-states-in-direct3d-12) and [Fence-Based Resource Management](https://learn.microsoft.com/en-us/windows/win32/direct3d12/fence-based-resource-management). | Barriers describe execution/memory dependencies, access scopes, image layouts, and potentially queue-family ownership transfers. See Vulkan [Synchronization and Cache Control](https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#synchronization). | Represent usage and producer/consumer dependencies, subresource scope, image layout needs, and queue dependencies. Let each backend lower them into native barriers. Distinguish GPU ordering/visibility from CPU-observed completion. |
| **Presentation** | `CAMetalLayer` supplies drawables; render to a drawable texture and present it through a command buffer. See [CAMetalLayer](https://developer.apple.com/documentation/quartzcore/cametallayer), [CAMetalDrawable](https://developer.apple.com/documentation/metal/mtldrawable), and [MTLCommandBuffer presentation](https://developer.apple.com/documentation/metal/mtlcommandbuffer/present(_:)). | DXGI swap chains provide back buffers, present, and resize behavior. See [DXGI flip model](https://learn.microsoft.com/en-us/windows/win32/direct3ddxgi/dxgi-flip-model) and [IDXGISwapChain::Present](https://learn.microsoft.com/en-us/windows/win32/api/dxgi/nf-dxgi-idxgiswapchain-present). | Window-system integration has surface/swapchain creation, image acquisition, and present operations with synchronization. See Vulkan [Window System Integration](https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#wsi). | Keep the host surface separate from swapchain lifecycle and acquired-image/present operations. Include extent, format/color space, image count, present mode, and acquire/present synchronization in the presentation description. |

## Proposed layers and interfaces

Namespaces below are illustrative. Every public class should have its own header; small enums/descriptors/value types belong in `GraphicsTypes.h`. Methods and exact result/error spelling should follow the existing engine error conventions when implementation begins.

### 1. Runtime, adapter, and device

- **`GraphicsInstance`** owns API-runtime setup when an API needs it, enumerates `Adapter` values, and supports validation/debug configuration. Metal may implement much of this as a lightweight adapter over system device discovery; Vulkan needs an instance.
- **`Adapter`** describes a selected physical GPU (name, vendor/device identifiers, queue families/roles, formats, limits, and optional features). It is a value/selection object, not a GPU resource.
- **`DeviceCapabilities`** is an immutable capability snapshot: supported formats/usages, sample counts, limits, shader features, queue roles, and enabled feature tiers.
- **`Device`** represents the logical GPU and owns factory operations for resources, pipelines, queues, and allocator services. Device creation takes an adapter and an explicit requested-feature set; unsupported required features fail visibly rather than silently downgrading.
- **`CommandQueue`** represents one queue role/queue family and accepts completed command buffers for submission. Do not assume graphics, compute, and transfer queues are either all distinct or all the same.

Suggested shape:

```cpp
class GraphicsInstance;
class Adapter;
class Device;

class GraphicsInstance {
public:
    std::vector<Adapter> EnumerateAdapters() const;
};

class Device {
public:
    const DeviceCapabilities &Capabilities() const noexcept;
    std::shared_ptr<Buffer> CreateBuffer(const BufferDesc &, Error &);
    std::shared_ptr<Texture> CreateTexture(const TextureDesc &, Error &);
    std::shared_ptr<CommandQueue> GetQueue(QueueType) const;
};
```

### 2. Recording, submission, and completion

- **`CommandContext`** is a short-lived, thread-confined recorder acquired for a queue/frame. It begins/ends scoped render, compute, and copy encoders and records resource references used by commands. It is not shared as an engine-wide mutable context.
- **`CommandBuffer`** is the ended/recorded command sequence submitted to a queue; it must not be reset/reused until backend completion rules allow it.
- **`CommandEncoder`** is the common recording scope; concrete **`RenderPassEncoder`**, **`ComputeEncoder`**, and **`CopyEncoder`** expose only valid operations for their scope.
- **`CommandQueue::Submit`** accepts a `CommandBuffer` and dependency waits/signals, and returns a **`Submission`** token identifying completion on that queue.
- **`Fence`** is the optional explicit CPU/GPU synchronization primitive for wait/query. Internal completion tracking should support per-queue timelines even if the public first slice only exposes `Submission`.

Submission does not itself mean work is complete. Recording retains shared references to resources; submitted work keeps those references alive through completion. Queue/allocator/pool backing storage and descriptor reuse also wait for their associated completion point.

### 3. Resources, views, and shared lifetime

- **`Resource`** is the polymorphic base for GPU-owned objects and contains backend identity plus lifetime bookkeeping. It has a virtual destructor and is non-copyable. Public references use `std::shared_ptr<T>`; raw native handles stay inside backend implementations.
- **`Buffer`** and **`Texture`** derive from `Resource`. A `BufferDesc` expresses size, usage/bind roles, and CPU access intent. A `TextureDesc` expresses dimension, format, extent, mip/layer/sample counts, and usage.
- **`BufferView`** and **`TextureView`** describe typed/ranged access to storage. A view is distinct from its backing resource because D3D12 descriptors and Vulkan image views have separate lifetimes and Metal's binding paths differ.
- **`Sampler`** is an independently owned immutable sampling configuration/resource.
- **`MemoryAllocator`** is a `Device` service that owns backend allocation strategy. Callers express intent and can inspect requirements; the API does not expose Metal heaps, D3D12 heaps, or `VkDeviceMemory` directly.
- Internal **`ResourceRetirementQueue`** holds native destruction payloads, descriptor slots, and transient allocations until every relevant queue completion point has passed.

#### Required lifetime rule

Shared-pointer reference count controls **CPU ownership**, not GPU completion. When a resource is recorded, its command buffer retains a shared reference. Submission keeps the recorded references until completion. If the final public/submission reference is released, the native payload is still retired against the last-use completion value; release happens only once all queue values that can use it have completed. This naturally may last longer than one frame and remains safe with multiple frames in flight or multiple queues. The same rule applies to views/descriptors, upload allocations, command allocators/pools, and swapchain images.

The backend/device state needed to destroy outstanding resources must outlive them. Use an internal shared device-state/retirement owner rather than letting a `Resource` keep a strong pointer to the public `Device` and create ownership cycles. Resource use tracking should be attached automatically by command recording/submission, not left to callers to remember to mark.

### 4. Shaders, bindings, and pipelines

- **`Shader`** represents a compiled backend-consumable shader artifact or selected entry point, with stage and reflection metadata. The interface should not force MSL, DXIL, or SPIR-V as one universal representation.
- **`BindingLayout`** declares binding index/type/count, shader visibility, and mutability/update policy. **`BindingSet`** supplies resources/views/samplers for a compatible layout.
- **`Pipeline`** is the shared base for immutable pipeline state. **`GraphicsPipeline`** includes shader stages, vertex input, topology, raster/depth/blend state, attachment formats, and sample count. **`ComputePipeline`** includes a compute shader and its layout.
- **`PipelineLayout`** should be a distinct class only if the first render path proves it useful; otherwise keep layout composition in `BindingLayout` and the pipeline descriptor. Vulkan/D3D12 make layout objects explicit, while Metal's encoding model is different.

Pipeline objects should be created before command recording and bound by encoders. Shader compilation, asset conversion, reflection, and cache policy belong in a distinct shader workflow and should not be smuggled into `Device::CreatePipeline`.

### 5. Render attachments and presentation

- **`RenderPassEncoder`** begins from a value `RenderPassDesc` containing color/depth attachments, load/store actions, clear values, and render extent. Make attachment compatibility visible to `GraphicsPipeline`.
- Do not bake a Vulkan-style render-pass object into the universal API merely because one native API provides it. A pass description/encoder can lower to Metal render encoders, D3D12 output-merger state, or Vulkan render-pass/dynamic-rendering facilities.
- Keep the existing host-facing **`Surface`** concept at the engine/OS boundary. Add **`Swapchain`** (or `PresentationChain`) for image acquisition, resize/recreation, and present policy. An acquired image is a temporary frame-scoped reference, not an ordinary user-owned texture.
- Presentation configuration includes pixel format/color space, extent, image count, present mode/latency preference, and acquire/present synchronization. Zero-size/minimized surfaces and out-of-date/recreation paths must be explicit.

### 6. Resource state and barriers

- **`ResourceUsage`**, **`AccessMode`**, stage masks, subresource ranges, and attachment/present roles are portable intent values in `GraphicsTypes.h`.
- **`ResourceBarrier`** describes a dependency between producer and consumer uses, including affected resource/subresource and optional queue ownership transfer. It is not just a generic `oldState -> newState` enum.
- **`ResourceStateTracker`** is initially backend-internal: it tracks uses and emits valid native transitions/hazards. This avoids burdening the first caller with Vulkan stage/access/layout detail while leaving room for explicit barriers and render-graph validation.
- Distinguish memory/ordering barriers from `Submission`/`Fence` completion. A completed CPU wait does not replace a needed in-GPU dependency, and a barrier does not prove that the CPU can reuse an allocation.

## Proposed file and class inventory

This is the target interface layout. The first portable abstract interfaces have been added under `engine/Graphics/`; no GPU implementation is implied. Each class is in its own header; add a `.cpp` only for portable implementation that is not purely abstract. Descriptor/value structs may share `GraphicsTypes.h`.

### Portable public core

| Proposed file | Class or contents | Responsibility |
|---|---|---|
| `engine/Graphics/GraphicsTypes.h` | Enums and value types (`QueueType`, `ShaderStage`, `BufferDesc`, `TextureDesc`, formats/usages, binding and pass descriptions, viewport, resource ranges, barrier intents) | Shared API vocabulary and descriptors; no backend handles. |
| `engine/Core/Mesh.h` | `Mesh`, `VertexStream` | GPU-buffer-backed vertex streams and pending upload dependencies for draw submission. |
| `engine/Core/MeshMaker.h` | `MeshMaker` | Create the initial non-indexed UV triangle through portable buffer/copy interfaces. |
| `engine/ShaderLibrary.h` | `ShaderLibrary`, `GraphicsShaderPair` | Placeholder named shader-pair request seam; it does not implement shader lookup or compilation. |
| `engine/Graphics/GraphicsInstance.h` | `GraphicsInstance` | Runtime setup and adapter enumeration. |
| `engine/Graphics/PresentationSurface.h` | `PresentationSurface` | Typed host presentation target used to create a swapchain without passing a raw native handle. |
| `engine/Graphics/Adapter.h` | `Adapter` | Physical GPU identity and selection data. |
| `engine/Graphics/DeviceCapabilities.h` | `DeviceCapabilities` | Feature, format, queue, and limit queries. |
| `engine/Graphics/Device.h` | `Device` | Logical GPU, capability access, resource/queue/pipeline factories, and typed-surface swapchain creation. Namespaced as `Engine::Graphics`; the former lifecycle `Engine::Device` has been removed. |
| `engine/Graphics/CommandQueue.h` | `CommandQueue` | Queue role and command submission. |
| `engine/Graphics/CommandContext.h` | `CommandContext` | Scoped command-recording facade. |
| `engine/Graphics/CommandBuffer.h` | `CommandBuffer` | Recorded work and completion-safe reset/reuse. |
| `engine/Graphics/CommandEncoder.h` | `CommandEncoder` | Common scoped encoder contract. |
| `engine/Graphics/RenderPassEncoder.h` | `RenderPassEncoder` | Attachment setup and graphics draw recording. |
| `engine/Graphics/ComputeEncoder.h` | `ComputeEncoder` | Compute dispatch recording. |
| `engine/Graphics/CopyEncoder.h` | `CopyEncoder` | Buffer/texture copy and upload operations. |
| `engine/Graphics/Submission.h` | `Submission` | Queue completion token and dependency representation. |
| `engine/Graphics/Fence.h` | `Fence` | Explicit wait/query synchronization where needed. |
| `engine/Graphics/Resource.h` | `Resource` | Shared-owned polymorphic base and lifetime identity/bookkeeping. |
| `engine/Graphics/Buffer.h` | `Buffer` | GPU buffer interface. |
| `engine/Graphics/Texture.h` | `Texture` | GPU image/texture interface. |
| `engine/Graphics/BufferView.h` | `BufferView` | Typed/ranged buffer access. |
| `engine/Graphics/TextureView.h` | `TextureView` | Image subresource and format view. |
| `engine/Graphics/Sampler.h` | `Sampler` | Sampling configuration. |
| `engine/Graphics/MemoryAllocator.h` | `MemoryAllocator` | Backend memory strategy and allocation intent/requirements. |
| `engine/Graphics/MemoryAllocation.h` | `MemoryAllocation` | Shared lifetime handle for an allocator-owned allocation. |
| `engine/Graphics/Shader.h` | `Shader` | Compiled shader artifact/entry point and reflection metadata. |
| `engine/Graphics/BindingLayout.h` | `BindingLayout` | Resource binding contract. |
| `engine/Graphics/BindingSet.h` | `BindingSet` | Resources bound to a compatible layout. |
| `engine/Graphics/Pipeline.h` | `Pipeline` | Shared immutable pipeline base. |
| `engine/Graphics/GraphicsPipeline.h` | `GraphicsPipeline` | Graphics shader and fixed-function state. |
| `engine/Graphics/ComputePipeline.h` | `ComputePipeline` | Compute shader state. |
| `engine/Graphics/ResourceBarrier.h` | `ResourceBarrier` | Portable synchronization dependency description. |
| `engine/Graphics/Swapchain.h` | `Swapchain` | Acquired frame image and presentation lifecycle. |
| `engine/Graphics/Detail/ResourceRetirementQueue.h` | Internal `ResourceRetirementQueue` | Completion-based destruction and reuse; not part of the user-facing API. |

No new public class file is proposed for adapter capabilities, usage flags, load/store operations, surface size, or format descriptions; those remain value types. Avoid creating a concrete class for every buffer role (vertex/index/uniform) until behavior requires one—use usage flags/views where native APIs share the same storage object.

The portable API now includes `PresentationSurface`, `Device::CreateSwapchain`, pipeline binding-layout association, and explicit render-pass viewport state. Graphics queues are required to support copy encoders so this initial frontend can stage its mesh without requiring a separate copy queue; queue submission dependencies express upload-before-draw ordering. The Engine frontend records a three-vertex position/UV mesh draw and asynchronous upload dependency using these declarations. These are contracts only: there is no backend implementation, and the API still does not implement GPU-completion retirement or establish native presentation behavior. Completion-based destruction remains mandatory when GPU objects are implemented.

### Backend implementation layout

Backend classes belong outside `engine/Graphics/` and never leak native types into portable headers. Split the current monolithic Metal implementation into per-class Objective-C++ modules under `backends/metal/Graphics/` as the Metal backend adopts the interfaces. Future D3D12 and Vulkan implementations follow the same ownership boundaries under `backends/d3d12/Graphics/` and `backends/vulkan/Graphics/`; their files are only created when those backends are approved.

Expected Metal implementation areas include instance/adapter selection, device, queue/context/command buffer, render/compute/copy encoders, buffer/texture/view/sampler, shader/pipeline/binding, allocator/retirement, and swapchain/presentation. The current `MetalRenderer.mm` is not an implementation of these interfaces; split or replace it when backend work begins. No claim is made that Metal needs a Vulkan-style global instance or identical objects for every interface.

## Recommended implementation order

1. **Frontend draw contract (implemented):** Engine selects an adapter from an injected `GraphicsInstance`, owns a device and swapchain, and records a triangle draw through the portable command API. The `Renderer` lifecycle dependency has been removed from Engine.
2. **Backend first render path:** implement queue, command context/buffer, submission completion, `Resource`, `Buffer`, `Texture`, views, allocator intent, and completion-based retirement. Validate a clear plus depth-tested mesh before adding abstractions not required by that path.
3. **Backend draw contract:** implement shader-pair resolution, `BindingLayout`/`BindingSet`, graphics pipeline, render-pass encoder, and swapchain/presentation; validate resize and device/surface failure behavior.
4. **Compute and portability proof:** add compute/copy encoders and explicit resource dependency tracking when the first real feature needs them. Validate the API against a small D3D12 and Vulkan mapping before claiming portability; record capability gaps instead of silently widening the common API.
5. **Follow-on services:** consider transient aliasing, timestamp/query pools, indirect draws, debug labels/validation, multi-queue transfer/compute, and render-graph scheduling as separately justified slices.

## Additional core concepts not named in the request

These are needed to make the requested pieces usable and correct:

- **Adapter discovery and capabilities:** formats, limits, feature tiers, and queue roles vary by GPU and API.
- **Command queues, command buffers, encoders, submissions, and fences:** needed to express modern recording/submission and asynchronous execution.
- **Resource views and subresource ranges:** storage and shader/render-target views are separate concerns in important backend paths.
- **Memory allocation and upload/readback policy:** CPU accessibility/coherency and GPU-local memory are not uniform.
- **Binding layouts/sets and samplers:** resource binding is a first-class system, not a property of shader objects alone.
- **Render attachment/pass descriptions and swapchain:** rendering targets, acquire/present, color space, resize, and minimized windows must be represented.
- **Viewport and pipeline binding-layout association:** a render pass needs an explicit viewport, and a graphics pipeline must identify the resource layout its bindings follow.
- **Resource usage and barriers/state tracking:** backends require different synchronization and image-layout handling.
- **Completion-aware resource retirement:** applies to resources and transient GPU-owned state, not just C++ object destruction.
- **Device loss, validation/debug labels, queries/timestamps, indirect drawing, and transient aliasing:** useful, but defer their public API until a concrete feature and test require them.

## Single-mesh frontend validation

`Engine::Draw` now models the intended path: acquire a `SwapchainImage`, use its
view as the color attachment, bind a graphics pipeline and transform uniform,
set the full-surface viewport, bind separate position and UV buffers, draw
three vertices, submit with the mesh-upload submission as a dependency, and
present. `MeshMaker` stages the equilateral triangle on the Graphics queue;
the mesh retains staging resources and its submission until completion is
observed. The transform block contains identity world/view/projection
matrices, while the shader pair is requested by program type and name through
an injected abstract service.

The validation uses portable fake interfaces only. It proves descriptor,
stream, command-order, dependency, resize, and error-reporting behavior; it
does not prove GPU visibility, shader/constant-buffer layout, copy support on a
Graphics queue, or presentation semantics. The placeholder shader library has
no catalog, compiler, reflection, or implementation. The portable
`PresentationSurface` currently exposes drawable extent only; backend surface
identity remains private to the implementing object and requires a concrete
backend contract before rendering can work.

## Research limitations

The source set is first-party Apple Developer Documentation, Microsoft Learn/Win32 API documentation, and the Khronos Vulkan Specification. Apple's canonical API-reference pages were available but some retrievals returned only symbol titles; Metal citations therefore point to the exact API symbols/methods rather than quoted prose. The Microsoft command-submission page explicitly states D3D12 has no immediate context and uses recorded command lists submitted to queues. The Vulkan specification provides the authoritative rules for device/queue setup, resources/memory, synchronization, pipelines/descriptors, and WSI. No secondary sources were used.
