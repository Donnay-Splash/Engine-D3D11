# Technical Design: Portable Engine with Metal First

## Purpose

The macOS target currently builds an Objective-C++ host that creates an AppKit window, a MetalKit view, and a Metal triangle renderer in one executable. The original Windows engine is a D3D11-specific static library. This document defines the intended engine/app split and a staged route from the triangle proof to selected original rendering features on a new Metal backend.

This is the architecture source of truth for the migration. It supersedes the former macOS Metal engine roadmap. It is a design and migration plan, not an implementation specification for every stage; each stage should get its own implementation spec before work begins.

## Goals

- Build the engine as a C++ static library that an application links against.
- Keep a genuinely portable C++ engine core that builds without Apple frameworks, Windows headers, D3D, or graphics-backend types.
- Keep renderer-specific implementation behind a small engine-level renderer interface. Metal is the first and only implemented backend.
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
macOS host/surface adapter (.mm; Apple-specific)
  - translates host surface lifecycle and drawable/resize events
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
Backend-neutral renderer interface
              ^
              |
Metal renderer adapter (.mm/C++ and MSL; internal implementation)
  - Metal device, queue, resources, shader/pipeline state, command encoding,
    render targets, and presentation
```

The exact CMake target names and whether the Metal implementation is a transitive static target or part of a final static engine target are implementation details. The required property is that the portable core can be built independently, while the macOS application links the engine implementation it needs. Keep Objective-C/Objective-C++, AppKit, Metal, and MetalKit out of the portable core and its public headers.

The renderer interface should describe engine-level rendering work and the capabilities actually needed by migrated features. It should not expose Metal or D3D types, require every Metal concept to have a one-to-one generic wrapper, or expose backend resource handles through the core. Keep internal seams inside the renderer implementation when they help tests; do not enlarge the app-facing interface for those seams.

The interface should be reviewed against likely D3D12/Vulkan concerns as implementation choices arise, especially resource lifetime, command submission, synchronization, shader stages, and render-target formats. That review is a constraint on avoidable coupling, not a requirement to invent a multi-backend framework before a second backend exists.

### Ownership and startup flow

1. The host creates the AppKit application, window, and native render surface/view.
2. The host calls the macOS-specific adapter to connect that surface to the engine. This is the platform-specific initialization seam; the portable engine interface remains C++ and backend-neutral.
3. The engine/Metal implementation creates and owns the Metal device-side rendering state, shaders, resources, and presentation work. Surface attachment must not move GPU resource ownership into the app.
4. The host runs the native AppKit event loop. The engine layer owns frame rendering/scheduling through the attached surface; the app does not call a D3D-style `Frame()` method every iteration.
5. Surface resize/drawable lifecycle is translated by the macOS adapter and delivered to the engine renderer. The exact host view type and resize callback contract are implementation decisions for the first library-backed rendering slice.
6. Shutdown stops rendering and releases engine resources before the host tears down its window and surface.

Do not copy the old swap-chain-created callback mechanically. The new handshake is host-created surface into engine initialization/attachment. If the selected surface technology requires callbacks for resize, drawable acquisition, or errors, keep those callbacks in the macOS adapter rather than the portable core.

Input can be added later as platform-neutral engine input values populated by a host adapter. It is not part of the initial rendering contract.

### App-facing interface qualities

The app-facing interface should stay small: configure/create the engine, attach the platform surface through its adapter, shut down, and receive explicit initialization/runtime failures. Exact names, parameter types, and whether construction returns a result or uses a factory are intentionally left for the first implementation spec.

The interface must state lifecycle ordering, surface ownership, resize behavior, frame scheduling, and failure reporting; a C++ type declaration alone is not the whole interface. Initialization failures (for example, unavailable device or invalid shader/pipeline) must be visible to the host rather than producing a success-shaped blank window.

## Build and platform structure

- Use CMake for the macOS engine and application; do not require `.sln` or `.vcxproj` files to configure/build them.
- Produce a static engine library. The old Windows Engine project is also a static library linked by its app; this is the expected packaging model for the first macOS engine.
- Separate a portable C++ core target from the Metal/macOS implementation targets. The macOS executable links the required engine target(s), and owns only the thin AppKit bootstrap/window wiring.
- Keep the Metal shader build, resource packaging, and Apple framework linkage with the Metal/macOS targets. Do not make the portable target depend on them.
- Keep the existing command-line bootstrap distinct from the GUI app unless a later decision deliberately removes it.
- Do not include legacy D3D11 sources or Windows libraries in the new CMake target source/link lists.

The current validated local baseline is Apple Silicon arm64, macOS 26.6.1, Xcode 26.6, macOS SDK 26.5, CMake 4.4.4, GNU Make 3.81, and Metal Toolchain 17F109. macOS 13.1 is the current provisional deployment floor, not a support promise; revalidate the baseline and deployment target when implementation starts. The current CMake setup uses:

```sh
cmake -S . -B build/macos -G "Unix Makefiles" \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=13.1
cmake --build build/macos
```

AppKit, Metal, and MetalKit come from the Apple SDK. The local workstation has already built and run the host-level MSL triangle; a packaged-shader offscreen check confirmed rendered pixels differ from the clear color. These checks validate the host/shader path only, not engine integration. See the README for current configure/build/run instructions.

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

### 1. Move the triangle behind the engine library

- Add the static engine/core and macOS/Metal adapter build separation.
- Move Metal device-side setup, shader/pipeline ownership, frame encoding, and presentation out of the app host.
- Let the host create the window/surface, attach it through the macOS adapter, and run AppKit.
- Prove library initialization, explicit failure reporting, resize, render scheduling, and shutdown.

**Acceptance:** the separate app links the engine library and displays the existing triangle; host source contains no shader, pipeline, command-encoding, or engine-resource implementation.

### 2. Establish the portable core and math

- Inventory all engine headers/sources, shader passes, Windows services, and build dependencies.
- Split portable data/behavior from renderer ownership, beginning with scene, component, property, mesh, and asset concepts.
- Replace DirectXMath dependencies on migrated engine-facing types and add tests for rendering-critical math conventions.
- Keep only necessary platform services behind small adapters.

**Acceptance:** the core builds on macOS without Apple graphics frameworks, Windows/D3D headers, DXGI, or DirectXMath; migrated math and data behavior has focused tests.

### 3. Render engine-owned scene content

- Define the first engine-owned CPU mesh/material/camera data contract and the minimum renderer operations it needs.
- Create Metal buffers, texture resources, and pipeline state only in the Metal implementation.
- Render a mesh selected/created through the engine scene path, with camera transforms and depth.
- Make shader compilation/loading an explicit CMake/runtime path with useful diagnostics.

**Acceptance:** engine-owned scene data, not hard-coded host geometry, produces a visible Metal-rendered mesh.

### 4. Select and migrate original rendering features

- Complete the feature-by-feature inventory: algorithm, data inputs/outputs, D3D11 assumptions, prerequisites, disposition, and visible/testable result. The current source-level inventory and first-effect recommendation are in [Legacy Engine Feature Inventory](legacy-feature-inventory.md).
- Stabilize basic mesh shading, materials, textures, depth, and render-target/resource lifetime first.
- Prototype shader authoring/conversion using representative vertex, fragment, and compute/stage needs; measure correctness and tooling before adopting it.
- Add render-target, compute, synchronization, and multi-pass capabilities only when a selected effect needs them.
- Port Deep G-Buffer and screen-space effects as separate vertical slices, prioritizing dissertation-critical results. Compare representative output with the legacy renderer where meaningful.
- Keep font/text/UI, asset conversion, and other legacy features deferred unless explicitly re-prioritized.

**Acceptance:** every migrated feature has a specific visible or testable outcome, and every unported feature is recorded as deferred or dropped rather than implied to work.

### 5. Add host services and assess additional backends

- Add input forwarding only when the rendering/scene milestone needs it; pass platform-neutral values, not AppKit events.
- Reassess clock, file, and diagnostics adapters as actual migrated features require them.
- After the Metal renderer is established, evaluate whether D3D12 or Vulkan is a real requirement and whether the renderer interface needs a deliberate revision.

**Acceptance:** no host-specific event or OS type leaks into the portable core; future backend work starts from an explicit requirement and validated interface changes.

### 6. Retire D3D11 deliberately

- Compare the source-level reuse inventory against migrated, intentionally dropped, and still-needed reference behavior.
- Identify the Win32, WinRT, and utility projects that stop building if D3D11 source/projects are removed.
- Confirm the intended loss of those Windows/D3D11 targets and preserve any required data, algorithms, or documentation first.
- Perform source/project deletion in a separate, explicit cleanup spec after review; excluding D3D11 from the new CMake build is required much earlier and is not itself permission to delete it.

**Acceptance:** retirement has an explicit impact record, all removed capabilities have a disposition, and no required migration reference is lost prematurely.

## Verification strategy

- **Build separation:** configure/build the portable core independently; audit its source and linked dependencies for Apple, Windows, D3D, DXGI, and DirectXMath leakage.
- **Library integration:** build the static engine implementation and a separate host executable that links it.
- **Runtime lifecycle:** verify app launch/close, resize, surface/drawable availability, device creation, clear/pipeline setup, triangle and mesh presentation, and repeated launch.
- **Failure paths:** exercise or simulate missing device, shader compilation/load failure, pipeline/resource allocation failure, and surface loss; ensure errors reach the host.
- **Math/data tests:** cover transforms, projection, coordinate conventions, matrix layout, and CPU-to-shader structures before dependent rendering work.
- **Feature tests:** validate each selected pass/effect independently and compare representative results to D3D11 where meaningful.
- **Retirement checks:** record platform projects and capabilities affected before removing old Windows/D3D11 source.

## Risks and decisions to make in implementation specs

- The exact public C++ lifecycle/result types and the exact macOS adapter signature remain open; keep the public surface small and specify ordering, ownership, resize, and errors.
- Choose whether the host supplies an `MTKView` or a simpler AppKit view/surface that the adapter configures. Preserve host ownership of the window while keeping Metal device and render state in the engine layer.
- Renderer coupling is broad: scene nodes, components, resources, pass ordering, and shader binding currently include D3D11 concepts. Do not mistake swapping `D3DClass` for a completed port.
- Matrix conventions and shader-facing layouts can change visible results. Test them before moving complex rendering passes.
- HLSL conversion may be lossy or have stage/resource limitations. Keep the workflow provisional until representative shaders compile and match behavior.
- The current arm64/macOS 13.1 baseline is not a release support commitment. Revisit hardware, deployment target, and tool versions before publishing support claims.
- Preserve the existing Windows projects during the migration, but do not present them as a supported future renderer. D3D12 and Vulkan remain future candidates, not current work.
