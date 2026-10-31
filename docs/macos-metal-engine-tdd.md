# Technical Design: Portable Engine with Metal First

## Purpose

The initial macOS prototype combined the AppKit host, MetalKit surface, and triangle renderer in one Objective-C++ executable. Issue #2 establishes the first library-backed slice: a separate AppKit host links a static engine implementation, with a portable C++ lifecycle core and an isolated Metal surface adapter. The original Windows engine remains a D3D11-specific static library. This document defines the intended engine/app split and a staged route from the triangle proof to selected original rendering features on a new Metal backend.

This is the architecture source of truth for the migration. It supersedes the former macOS Metal engine roadmap. It is a design and migration plan, not an implementation specification for every stage; each stage should get its own implementation spec before work begins.

## Goals

- Build the engine as a C++ static library that an application links against.
- Keep a genuinely portable C++ engine core that builds without Apple frameworks, Windows headers, D3D, or graphics-backend types.
- Build a portable graphics interface for device, resources, command submission, pipelines, and presentation; keep backend implementations isolated. Metal is the first and only implemented backend.
- Let the host own the native window and event loop while the engine owns graphics setup, engine-side rendering, and frame scheduling.
- Migrate original engine behavior selectively, prioritizing the dissertation-critical scene and rendering features, including Deep G-Buffer effects where feasible.
- Make future D3D12 and Vulkan evaluation possible without designing or implementing all backends in advance.
- Retire D3D11 after the needed behavior has been inventoried and the retirement impact reviewed.

## Non-goals

- Porting or maintaining D3D11 as a backend for the new engine.
- Implementing D3D12 or Vulkan as part of the initial Metal port.
- Guaranteeing one-to-one parity with every Windows/WinRT feature.
- Porting the Win32/WinRT application hosts, Direct2D/DirectWrite UI, or the DirectXTex-based asset converter without a specific requirement.
- Forwarding input through the new interface in the first rendering slice.
- Recreating D3D11 device, immediate-context, swap-chain, or resource APIs as a supposedly generic interface.

## Settled architecture

### Module and dependency layout

```text
macOS AppKit host
  - owns NSApplication, NSWindow, native render surface, and OS event loop
  - performs bootstrap and connects the surface to the engine
              |
              v
macOS frontend bridge (.mm; Apple-specific)
  - owns AppKit/MetalKit integration and exposes a narrow native-layer seam
              |
              v
Engine facade (C++; static link target)
  - small app-facing interface and engine lifecycle
              |
              v
Portable engine core (C++; independently buildable)
  - engine state, portable scene/data, math and frame-level behavior
              |
              v
Portable graphics API (C++)
  - GraphicsInstance, Adapter, Graphics::Device, Resource, commands,
    resources, bindings, pipelines, and presentation contracts
              ^
              |
Metal backend (.cpp/metal-cpp and MSL)
  - adapter/device, resources, shader/pipeline state, command encoding,
    render targets, synchronization, and drawable presentation
```

The portable core can be built independently, while the macOS application links the engine implementation it needs. Keep Objective-C/Objective-C++, AppKit, Metal, and MetalKit out of the portable core and its public headers. Metal backend implementation and platform integration use C++ and metal-cpp; Objective-C++ is confined to the macOS frontend bridge.

The portable graphics API describes shared rendering intent and required capabilities without exposing Metal or D3D types or backend resource handles. It should not force every native concept into a one-to-one wrapper. The former `Engine::Device` renderer-lifecycle interface is retired, and `Engine` no longer depends on the temporary `Engine::Renderer` bridge. The new GPU-facing type is `Engine::Graphics::Device`; the legacy renderer is not part of the active CMake build, and the Metal backend implements the component API.

The interface should be reviewed against likely D3D12/Vulkan concerns as implementation choices arise, especially resource lifetime, command submission, synchronization, shader stages, and render-target formats. That review is a constraint on avoidable coupling, not a requirement to invent a multi-backend framework before a second backend exists.

### Ownership and startup flow

1. The host creates the AppKit application, window, and native render surface/view.
2. The host calls the macOS-specific adapter to connect that surface to the engine. This is the platform-specific initialization seam; the portable engine interface remains C++ and backend-neutral.
3. Engine obtains a `GraphicsInstance` and shader library from its factory, selects the preferred or first graphics-capable adapter, creates its `Graphics::Device`, and owns the swapchain.
4. The host runs the native AppKit event loop. The engine layer owns frame rendering/scheduling through the attached surface; the app does not call a D3D-style `Frame()` method every iteration.
5. Surface resize is delivered through the host adapter; draw callbacks are notifications only. Engine acquires/presents swapchain images through the graphics API.
6. Shutdown stops rendering and releases engine resources before the host tears down its window and surface.

Do not copy the old swap-chain-created callback mechanically. The new handshake is host-created surface into engine initialization/attachment. If the selected surface technology requires callbacks for resize, drawable acquisition, or errors, keep those callbacks in the macOS adapter rather than the portable core.

Input can be added later as platform-neutral engine input values populated by a host adapter. It is not part of the initial rendering contract.

### App-facing interface qualities

The app-facing interface should stay small: configure/create the engine, attach the platform surface through its adapter, shut down, and receive explicit initialization/runtime failures. The `createEngine()`, `Engine::Initialize(Surface&, ErrorHandler, Error&)`, and `Engine::Shutdown()` lifecycle remains. `EngineFactory` injects a `GraphicsInstance` and placeholder shader-library service; Engine selects a graphics-capable adapter, creates its device, and owns the swapchain.

The interface must state lifecycle ordering, surface ownership, resize behavior, frame scheduling, and failure reporting; a C++ type declaration alone is not the whole interface. Initialization failures (for example, unavailable device or invalid shader/pipeline) must be visible to the host rather than producing a success-shaped blank window.

The host surface implements the typed `Graphics::PresentationSurface` contract and sends frame notifications; it does not acquire or pass a native drawable to portable Engine code. Engine owns swapchain acquisition, render-pass recording, submission, and presentation. The host retains ownership of the view and adapter, and the surface must outlive the engine because the engine keeps a non-owning attachment. `Engine::Shutdown` stops surface callbacks before releasing graphics resources.

The current frontend path creates a three-vertex mesh with separate position and UV buffers, requests a shader pair by program type/name, writes identity world/view/projection matrices to a uniform buffer, and records a draw of three vertices. Upload uses staging buffers and an explicit submission dependency. Portable fake graphics objects exercise this path; the macOS host connects it to Metal and schedules rendering on the AppKit event loop. Apple-only Metal integration tests execute graphics and compute work against actual Metal hardware, verify buffer-copy results, and exercise resources and synchronization. Those tests do not verify window presentation or substitute for running the app with Metal API Validation.

Metal API Validation is enabled through the generated Xcode development scheme using CMake's `XCODE_SCHEME_ENABLE_GPU_API_VALIDATION` property. It must be active before Metal device creation; Metal exposes no public device-level runtime query or switch for this diagnostic. Launch through the generated Xcode scheme during development. The backend cannot detect or assert that API Validation is active.

## Build and platform structure

- Use CMake for the macOS engine and application; do not require `.sln` or `.vcxproj` files to configure/build them.
- Produce a static engine library. The old Windows Engine project is also a static library linked by its app; this is the expected packaging model for the first macOS engine.
- Separate a portable C++ core target from the Metal/macOS implementation targets. The macOS executable links the required engine target(s), and owns only the thin AppKit bootstrap/window wiring.
- Keep the Metal shader build, resource packaging, and Apple framework linkage with the Metal/macOS targets. Do not make the portable target depend on them.
- Keep the existing command-line bootstrap distinct from the GUI app unless a later decision deliberately removes it.
- Do not include legacy D3D11 sources or Windows libraries in the new CMake target source/link lists.

The validated build baseline is Apple Silicon arm64, macOS 26.6.1, Xcode 26.6, macOS SDK 26.5, CMake 4.4.4, and Metal Toolchain 17F109. The CMake project currently targets macOS 26; revalidate the toolchain and deployment target before publishing support claims. The current CMake setup uses:

```sh
cmake -S . -B build/macos -G Xcode \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=26
cmake --build build/macos --target engine-macos engine-metal-integration-tests
ctest --test-dir build/macos -C Debug --output-on-failure
```

AppKit, Metal, and MetalKit come from the Apple SDK. The CMake build compiles and packages the MSL shader library and links the separate GUI host. CTest includes portable lifecycle and graphics tests plus a hardware-backed Metal integration test. Launch the app or integration-test target from its generated Xcode scheme to run Metal API Validation; invoking the test directly through CTest does not enable the Xcode scheme diagnostic setting. See the README for current configure/build/run instructions.

## Legacy feature inventory and disposition

This is the initial disposition. The first migration stage must refine it against the actual source, shaders, and dependencies; unlisted or uncertain behavior must not be silently treated as migrated.

| Existing area | Initial disposition |
|---|---|
| Scene graph, scene nodes, components, and public properties | Preserve portable behavior and data where useful; remove renderer/device ownership from scene types. |
| CPU-side mesh data and asset formats | Preserve where practical; separate vertex/index/material descriptions and loaders from D3D11 buffers and views. |
| D3D11 device/swap chain, immediate context, buffers, textures, render targets, samplers, shader wrappers, and pipeline state | Replace inside the Metal renderer; do not wrap D3D11 objects or expose backend handles through the core. |
| Scene lighting, environment loading/lighting, light management, and shadow-map rendering | Preserve useful scene/light data; redesign GPU resources and shadow passes for Metal after the basic camera/material path is stable. |
| Deep G-Buffer generation/shading, Lambertian pass, Hi-Z, ambient occlusion, radiosity, temporal filtering/TSAA, blur, and tone mapping | Retain as algorithm/reference material; prioritize dissertation-critical passes after an engine-owned basic scene renders. Validate each pass and record any redesign or omission. |
| DirectXMath-based vectors, matrices, colors, and bounds | Replace or isolate behind portable math values. Test transforms, projection, multiplication order, handedness, storage/layout, and CPU-to-shader data before large shader migrations. |
| HLSL shaders and shader pipeline definitions | Preserve algorithm intent/source as reference. Start with MSL where useful for Metal bring-up; evaluate HLSL-to-MSL and optional SPIR-V workflows before committing to a shared authoring strategy. |
| Geometry-shader assumptions and D3D11 pass/resource semantics | Identify per feature and redesign for Metal; do not assume a direct stage/API mapping. |
| Windows timing, input, file access, and diagnostics | Extract only the operations the portable engine needs, behind platform-neutral types/adapters. Input forwarding is deferred beyond the first rendering milestone. |
| D3D11 font/text rendering, Direct2D/DirectWrite UI, Win32/WinRT hosts | Defer or drop unless a concrete requirement justifies a separate replacement. |
| DirectXTex-based AssetConverter | Defer. Record required asset formats and choose a portable conversion route only when needed. |
| D3D11 engine and dependent Windows projects | Keep as temporary reference during migration; do not port. Gate removal on the retirement checklist below. |
| D3D12 and Vulkan | Future evaluation targets only. Do not implement them or add speculative abstraction solely for them. |

The current monolithic `Engine::Engine` coordinates input, scene state, and a D3D11-specific pipeline. Existing `Scene`, `Mesh`, and `Material` interfaces also take D3D11 device/context types. Extract behavior and CPU-side concepts; do not attempt to compile the old `Engine` unchanged as the portable core.

## Migration stages

Each stage should be turned into a focused implementation spec with its own files, scope, tests, and completion criteria.

### 1. Establish the host and surface lifecycle

- Add the static engine/core and macOS/Metal adapter build separation.
- Let the host create the window/surface, attach it through the macOS adapter, and run AppKit.
- Keep initialization, explicit failure reporting, resize, and shutdown independent of rendering work.

**Acceptance:** the separate app links the engine library and handles surface startup, resize, failure, and shutdown; rendering is not required at this stage.

### 2. Establish the portable core and math

- Inventory all engine headers/sources, shader passes, Windows services, and build dependencies.
- Split portable data/behavior from renderer ownership, beginning with scene, component, property, mesh, and asset concepts.
- Replace DirectXMath dependencies on migrated engine-facing types and add tests for rendering-critical math conventions.
- Keep only necessary platform services behind small adapters.

**Acceptance:** the core builds on macOS without Apple graphics frameworks, Windows/D3D headers, DXGI, or DirectXMath; migrated math and data behavior has focused tests.

The first portable-core slice exposes `Core/Math.h` and `Core/SceneData.h` from
`engine-core`. Its matrices use row-major storage and row-vector multiplication;
the perspective helper preserves the legacy left-handed, zero-to-one depth
convention. CPU scene nodes refer to mesh/material value data, meshes retain
vertex attributes and 32-bit indices, and bounds can be calculated and
transformed without renderer handles. This does not establish a portable file
format, GPU constant-buffer layout, editor property system, or complete scene
graph; those remain subject to the inventory and later migration specs. The
core target is constrained to C++ sources with no linked platform/backend
libraries, and can be built directly as `engine-core`.

### 3. Establish the modular graphics interfaces

- Define backend-neutral interfaces for instance/adapter/device, resources/views, queues/command recording/submission, shaders/bindings/pipelines, allocation, barriers, and presentation.
- Use shared CPU ownership for GPU resources and require completion-based native retirement after final GPU use.
- Keep `Engine` on the graphics component interfaces; do not add backend handles to portable headers.
- Compile all public headers and test shared ownership without introducing a rendering dependency.

**Acceptance:** portable interfaces compile and remain independent of platform graphics libraries; a fake-driven frontend draw path validates the contracts, but no backend implementation or on-screen rendering is required.

### 4. Implement the first Metal backend and render engine-owned scene content

- Define the first engine-owned CPU mesh/material/camera data contract and map it to the portable graphics interfaces.
- Implement the Metal-specific device, queue, resource, shader, binding, pipeline, and presentation components incrementally.
- Render a mesh selected/created through the engine scene path, with camera transforms and depth.
- Make shader compilation/loading an explicit CMake/runtime path with useful diagnostics.

**Current implementation:** the engine-owned UV triangle is rendered through the Metal graphics backend, and the hardware-backed integration test exercises graphics and compute submission. The app builds with a generated API-Validation-enabled Xcode scheme. Window presentation should still be exercised on the target display through that scheme.

The next scene slice keeps the portable structure-of-arrays mesh contract
(`positions`, optional vertex attributes, and 32-bit indices), adds a portable
camera view transform and left-handed perspective parameters, and builds one
engine-owned cube scene. The engine passes its CPU scene and camera into the
internal backend; Metal owns the vertex/normal/index buffers, shader pipeline,
depth state, and depth texture. This slice intentionally renders one scene object;
multi-object/material
pipelines and a general scene traversal contract remain future work.

### 5. Select and migrate original rendering features

- Complete the feature-by-feature inventory: algorithm, data inputs/outputs, D3D11 assumptions, prerequisites, disposition, and visible/testable result. The current source-level inventory and first-effect recommendation are in [Legacy Engine Feature Inventory](legacy-feature-inventory.md).
- Stabilize basic mesh shading, materials, textures, depth, and render-target/resource lifetime first.
- Prototype shader authoring/conversion using representative stages supported by the source inventory; see the [shader workflow evaluation](shader-workflow-evaluation.md) for the compiler evidence, provisional direct-MSL recommendation, and conversion constraints. Reassess when a concrete migrated feature or additional backend requires it.
- Add render-target, compute, synchronization, and multi-pass capabilities only when a selected effect needs them.
- Port Deep G-Buffer and screen-space effects as separate vertical slices, prioritizing dissertation-critical results. Compare representative output with the legacy renderer where meaningful.
- Keep font/text/UI, asset conversion, and other legacy features deferred unless explicitly re-prioritized.

**Acceptance:** every migrated feature has a specific visible or testable outcome, and every unported feature is recorded as deferred or dropped rather than implied to work.

### 6. Add host services and assess additional backends

- Add input forwarding only when the rendering/scene milestone needs it; pass platform-neutral values, not AppKit events.
- Reassess clock, file, and diagnostics adapters as actual migrated features require them.
- After the Metal backend is established, evaluate whether D3D12 or Vulkan is a real requirement and whether the portable graphics interfaces need a deliberate revision.

**Acceptance:** no host-specific event or OS type leaks into the portable core; future backend work starts from an explicit requirement and validated interface changes.

### 7. Retire D3D11 deliberately

- Compare the source-level reuse inventory against migrated, intentionally dropped, and still-needed reference behavior.
- Identify the Win32, WinRT, and utility projects that stop building if D3D11 source/projects are removed.
- Confirm the intended loss of those Windows/D3D11 targets and preserve any required data, algorithms, or documentation first.
- Perform source/project deletion in a separate, explicit cleanup spec after review; excluding D3D11 from the new CMake build is required much earlier and is not itself permission to delete it.

**Acceptance:** retirement has an explicit impact record, all removed capabilities have a disposition, and no required migration reference is lost prematurely.

## Verification strategy

- **Build separation:** configure/build the portable core independently; audit its source and linked dependencies for Apple, Windows, D3D, DXGI, and DirectXMath leakage.
- **Library integration:** build the static engine implementation and a separate host executable that links it.
- **Runtime lifecycle:** verify app launch/close, resize, surface/drawable availability, device creation, clear/pipeline setup, triangle and mesh presentation, and repeated launch. For the first scene slice, confirm the transformed, depth-tested engine mesh remains visible after resizing and relaunching.
- **Failure paths:** exercise or simulate missing device, shader compilation/load failure, pipeline/resource allocation failure, and surface loss; ensure errors reach the host.
- **Math/data tests:** cover transforms, projection, coordinate conventions, matrix layout, and CPU-to-shader structures before dependent rendering work.
- **Feature tests:** validate each selected pass/effect independently and compare representative results to D3D11 where meaningful.
- **Retirement checks:** record platform projects and capabilities affected before removing old Windows/D3D11 source.

## Risks and decisions to make in implementation specs

- The exact public C++ lifecycle/result types and the exact macOS adapter signature remain open; keep the public surface small and specify ordering, ownership, resize, and errors.
- The first slice uses a host-created `MTKView` behind an Objective-C++ surface adapter. Preserve host ownership of the window and view while keeping Metal device and render state in the engine layer.
- Renderer coupling is broad: scene nodes, components, resources, pass ordering, and shader binding currently include D3D11 concepts. Do not mistake swapping `D3DClass` for a completed port.
- Matrix conventions and shader-facing layouts can change visible results. Test them before moving complex rendering passes.
- HLSL conversion may be lossy or have stage/resource limitations. Keep the workflow provisional until representative shaders compile and match behavior.
- The current arm64/macOS 26 baseline is not a release support commitment. Revisit hardware, deployment target, and tool versions before publishing support claims.
- Preserve the existing Windows projects during the migration, but do not present them as a supported future renderer. D3D12 and Vulkan remain future candidates, not current work.
