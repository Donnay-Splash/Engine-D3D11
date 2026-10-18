# Legacy Engine Feature Inventory

This inventory records what exists in the Windows/D3D11 code and what should
carry forward into the Metal-first engine. It is a source-level migration
record, not a claim that any listed feature has been ported or runtime-validated.
The target architecture and migration constraints are in the
[macOS Metal engine technical design](macos-metal-engine-tdd.md); the current
macOS executable now renders a starter mesh from engine-owned portable scene
data, as described in the [README](../README.md#macos-app-setup).

## Disposition key

- **Preserve** — retain the behavior or data concept in the portable core.
- **Adapt** — retain the capability but redesign its API, data representation,
  or platform boundary.
- **Replace for Metal** — implement the capability behind the engine renderer
  using Metal resources and commands, without carrying over D3D11 wrappers.
- **Defer** — keep as a migration candidate, but not a prerequisite for the
  first engine-owned scene/effect slices.
- **Drop** — do not carry forward absent a new requirement; this does not
  authorize deleting legacy source in this ticket.

## Scene, content, and services

| Area and current evidence | Dependencies and migration notes | Disposition |
| --- | --- | --- |
| **Scene graph and transforms.** `Scene` owns a root `SceneNode`; nodes keep parent/child links, local position/rotation/uniform scale, components, and transform state. `Scene::Update`/`Render`, component queries, bounds aggregation, and change notifications are implemented in [Scene](../Engine/Header%20Files/Scene/Scene.h), [SceneNode](../Engine/Header%20Files/Scene/SceneNode.h), and their source files. | The graph is conceptually CPU-side, but `Scene` stores a `D3DClass*`; node/component rendering signatures take D3D11 contexts and shader pipelines. Math types are DirectXMath-backed. Keep hierarchy, transform propagation, component ownership, and useful bounds behavior; remove renderer ownership from scene types and validate transform conventions before porting effects. | **Preserve** graph and transform behavior; **adapt** renderer-facing traversal and math. |
| **Components and editable properties.** Existing components include `Camera`, `MeshInstance`, `Light`, `BoundingBox`, `PostProcessingCamera`, and `ShadowMapCamera`. `SceneElement` exposes boolean, scalar, and vector properties through getters/setters and optional ranges. See [component headers](../Engine/Header%20Files/Scene/Components/Component.h) and [Property](../Engine/Header%20Files/Scene/Public%20Properties/Property.h). | Camera and render-target components currently own D3D-specific state; the scene-element/property model is independent in concept. `Engine_WinRT` projects legacy elements to C++/CX (`SceneElementCX`, `PropertyCX`) for the XAML property UI. Retain data needed for scene editing only if the new app needs it; do not port C++/CX bindings into the core. | **Adapt** CPU component/property concepts; **defer** the old editor integration and renderer-specific camera components. |
| **Mesh data and instances.** `Mesh` stores positions, normals, tangents/bitangents, UVs, and 32-bit indices; `MeshInstance` binds a mesh and material to a scene node and tracks previous transforms. `MeshMaker` creates a cube. See [Mesh](../Engine/Header%20Files/Resources/Mesh.h) and [MeshInstance](../Engine/Header%20Files/Scene/Components/MeshInstance.h). | `Mesh` mixes CPU arrays with D3D11 vertex/index buffers and device finalization. The data attributes needed by shaders and bounds are useful, but GPU buffers, upload policy, index format, and ownership belong to the Metal renderer. | **Preserve** needed CPU mesh and instance data; **replace for Metal** buffer creation, binding, and drawing. |
| **Materials and textures.** `Material` stores diffuse/specular/emissive colors, opacity, smoothness, and texture slots for diffuse, specular, emissive, normal, smoothness, opacity, and baked AO. See [Material](../Engine/Header%20Files/Resources/Material.h) and [Texture](../Engine/Header%20Files/Resources/Texture.h). | The material model is tied to D3D11 constant buffers, samplers, shader pipelines, and SRV registers. Texture creation also includes D3D11 format/array/mip flags and file loading. Preserve artist-facing values and semantic texture roles; redesign GPU formats, residency, sampler state, and binding for actual migrated passes. | **Preserve** material semantics; **replace for Metal** texture/resource ownership and binding. |
| **Scene and asset serialization.** `Utils::Loader::SceneData` contains node transforms/mesh arrays, material records, and embedded texture bytes; `MikeLoader` reads either a path or byte buffer and `Exporter` writes the data. `Engine::Loader` constructs D3D-backed scene objects from this model. See [serialized data](../Utils/Loader/Data.h), [MikeLoader](../Utils/Loader/MikeLoader.h), and [Engine Loader](../Engine/Source%20Files/Loader.cpp). | The current binary format has a version constant and record IDs, but its representation uses `DirectXMath`-based types and its reader asserts on bounds rather than returning structured errors. Preserve only after deciding versioning, validation, endian/layout rules, and CPU/GPU separation; do not equate it with a portable asset contract yet. | **Adapt** scene-data concepts and file/error boundary; **defer** broad importer support. |
| **Asset conversion.** `AssetConverter` has Assimp importer and texture manager sources and links `Utils`, Assimp, DirectXTex, Shlwapi, and Ole32 in its Visual Studio project. | This is a Windows utility, not part of the macOS CMake target. DirectXTex DDS conversion and Windows path/COM APIs are not reusable unchanged. Existing serialized texture/material/node data may inform a future replacement, but its format and required conversion features have not been fully validated here. | **Defer**; later choose a portable converter or explicit asset pipeline. |
| **Math and shader-facing data.** `Utils/Math/Math.h` wraps DirectXMath matrix/vector/quaternion/color types and provides transforms, projections, and bounds-related math. `ConstantBufferLayouts.h` defines aligned view/object/light/material and effect constants. | DirectXMath is included by shared data and shader-facing structures, not just isolated utility implementation. Matrix layout, multiplication order, projection/depth convention, jitter, and constant-buffer packing affect visible output. Establish and test those conventions before moving rendering algorithms; do not copy D3D constant-buffer alignment as the portable data model. | **Adapt** math to portable C++ types and explicit GPU layouts; preserve proven algorithms after parity tests. |
| **Timing, input, and performance.** `TimerClass` uses `QueryPerformanceCounter`; `fpsclass` uses Windows timing; `PerformanceTracker` uses PDH and `GetTickCount`. `InputManager` consumes an `InputState` with pointer position, mouse buttons, pressed/released key sets, and edge queries. | `Engine::Frame` currently polls time and processes input in an engine-owned frame call. The input value model can be populated by a host adapter, but the design defers input forwarding and expects the host's AppKit loop to own native events. Timing needs an explicit portable clock/scheduling seam; CPU-usage reporting is not render-critical. | **Adapt** a clock only when frame scheduling needs it; **defer** input and performance UI. |
| **Files and diagnostics.** Loading accepts path or byte data through `MikeLoader`; other file access uses standard streams. The legacy code relies heavily on `EngineAssert`; the inspected sources do not show a unified cross-platform file or diagnostic service. `EngineCreateOptions` also exposes Win32/XAML and swap-chain callback types. | The byte-buffer loader is a possible seam, but errors need explicit results rather than assertions for a host that must report Metal initialization/shader failures. Native paths and host callbacks should stay out of portable interfaces. Whether the migrated engine needs filesystem access or a logging subsystem is feature-dependent and not established by this inventory. | **Adapt** error reporting at boundaries; **defer** a general file service until an asset requirement exists. |
| **Threading and generic utilities.** `Utils` includes header-only `ThreadPool` and `ThreadSafeQueue` as well as string helpers. | Their current use by migrated render/asset paths was not established from the inspected engine entry points. They have no reason to be part of the graphics backend contract. | **Defer** until an asynchronous workload needs them; retain only utilities with demonstrated consumers. |

## Renderer and pass inventory

The old renderer is not a backend-neutral rendering interface. D3D11 types
appear in scene, component, material, mesh, camera, texture, pipeline, and
render-target APIs; `D3DClass` owns device/context/swap-chain operations. The
resource wrappers include constant/vertex/index buffers, input layouts,
shaders/pipelines, blend/depth/rasterizer states, samplers, textures, mip
views, render targets, and render-target bundles. See
[resource headers](../Engine/Header%20Files/Resources/Texture.h),
[RenderTargetBundle](../Engine/Header%20Files/RenderTargetBundle.h), and
[D3DClass](../Engine/Header%20Files/d3dclass.h).

| Feature | Existing prerequisites / observable result | Disposition |
| --- | --- | --- |
| **Base scene draw and depth.** Camera projection and view constants feed mesh vertex transforms; materials and meshes are submitted through D3D11 input layouts, buffers, pipeline state, and depth/render-target views. | This is the prerequisite for migrated scene content and every screen-space pass. Implement the smallest Metal-owned mesh/material/camera path first; test coordinate/depth conventions and resize. | **Replace for Metal** renderer resources and draw path; **preserve** only validated CPU data. |
| **Deep G-Buffer generation.** `Engine::ResizeBuffers` creates a two-slice bundle with `Main`, `Emissive`, `Normal`, `SSVelocity`, and camera-space `CSZ` targets. The generation pixel shader writes those five MRT outputs; the geometry shader emits each triangle for both layers. Generation also samples prior depth and computes screen-space velocity. See [Engine pass orchestration](../Engine/Source%20Files/Engine.cpp), [generation pixel shader](../Engine/Shaders/DeepGBuffer_Gen.ps.hlsl), [generation geometry shader](../Engine/Shaders/DeepGBuffer_Gen.gs.hlsl), and [generation bindings](../Engine/Shaders/DeepGBuffer_Gen_Globals.hlsl). | Requires multi-render-target output, a two-layer image/depth representation, camera/object history, texture sampling, and a plan for the D3D geometry-shader layer expansion. The existing shader uses previous depth to reject fragments for the second layer; first-frame/history reset behavior and robust layer separation need explicit validation. | **Preserve** the dissertation-critical two-layer result; **replace for Metal** the D3D11 pass and define a Metal-compatible layered-generation strategy. |
| **Camera-space Z copy and hierarchical Z.** `DeepGBuffer_CameraZCopy` reduces the two G-Buffer Z slices to two channels; `DeepGBuffer_Minify_CSZ` builds the mip chain. AO and radiosity sample this hierarchy at radius-selected mip levels. | Requires renderable/sampleable mip levels, mip-specific render targets, safe read/write transitions, and camera-space reconstruction constants. | **Replace for Metal** as a reusable depth-pyramid pass after the two-layer output exists. |
| **Screen-space ambient occlusion.** `DeepGBuffer_ComputeAO` samples both camera-space Z layers, reconstructs sample positions, applies AO radius/bias/sample controls, and can incorporate the second layer. `Engine::GenerateAO` then runs a vertical and horizontal depth-aware blur and `DeepGBuffer_Shade` applies AO to ambient lighting. | Depends on deep G-Buffer depth, Hi-Z, projection/reconstruction math, render-target sampling, and a separable blur. Observable result: adjustable local occlusion at contact/nearby geometry, with depth edges retained by bilateral filtering. Temporal AO accumulation is not currently in `RenderGraphics`. | **Recommend first effect vertical slice** (bounded criteria below); **replace for Metal** its fullscreen passes and resources. |
| **Lambertian prepass and screen-space radiosity.** `LambertianOnly` writes both layer colors and packed normals; `Minify_Lambertian` builds color/normal mips; `ComputeRadiosity` samples those and Hi-Z to estimate indirect light. `BasicTemporalFilter` reprojects using velocity and prior depth, then a bilateral blur runs; deferred shading consumes the result. See [radiosity shader](../Engine/Shaders/DeepGBuffer_ComputeRadiosity.ps.hlsl), [Lambertian shader](../Engine/Shaders/DeepGBuffer_LambertianOnly.ps.hlsl), and [temporal filter](../Engine/Shaders/DeepGBuffer_BasicTemporalFilter.ps.hlsl). | This is a multi-pass feature requiring layered depth/color, normals, mips, motion vectors, temporal history, intermediate targets, and filtering. The existing pass order and history invalidation need reference-image validation. It is more coupled than the first AO slice. | **Defer** until scene, deep G-Buffer, depth hierarchy, and AO/resource lifetimes are validated; then port as separate passes. |
| **Deferred direct/environment lighting and shadows.** `LightManager` gathers up to four lights, supports an environment cube map, and renders a shadow map for a selected shadow-casting light; deferred shading evaluates BRDF, environment lighting, shadow visibility, AO, and radiosity. See [LightManager](../Engine/Source%20Files/LightManager.cpp), [lighting shader includes](../Engine/Shaders/LightingConstants.hlsl), and [deferred shading](../Engine/Shaders/DeepGBuffer_Shade.ps.hlsl). | Requires G-Buffer material/normal data, light data, shadow depth + PCF, cube-map sampling, and HDR output. The D3D implementation has fixed limits and a D3D resource/register layout. Basic shading is part of establishing the scene path; shadow/environment parity is not needed to prove the first effect. | **Adapt** light/material semantics; **replace for Metal** when selected; **defer** environment and shadow parity beyond basic shading. |
| **Temporal super-sampling and tone mapping.** `RunTSAA` combines current HDR, previous frame, screen velocity, camera jitter, and a Catmull-Rom weighted 3×3 neighborhood. `Tonemap` maps HDR to LDR using the filmic shader. | Requires stable motion vectors, jittered projection, history storage/reset, HDR targets, and a presentation-compatible format. These are downstream from first scene/effect correctness. | **Defer**, then port as independent post-processing slices with image-based acceptance. |
| **Font/text and simple legacy shaders.** `Font`, `FontShader`, `Text`, and `color/texture/light/font .vs/.ps` files coexist with the newer HLSL shader family. The inspected current shader manager builds its core pipelines from the HLSL family. | The use/maintenance status of the older `.vs`/`.ps` files is unclear; font rendering also depends on D3D resources and legacy font texture/data. | **Defer**; determine whether these shader files and text paths are live before deciding to port or drop them. |

### First effect slice: Deep-G-Buffer-aware AO

Build the two-layer depth input and camera-space reconstruction needed by the
existing AO algorithm, then add AO and its depth-aware blur as the first
screen-space effect. Keep radiosity, shadow maps, environment lighting, temporal
AA, and broad asset importing out of this slice.

**Prerequisites**

1. Engine-owned mesh/camera rendering with tested view/projection, depth, and
   resize behavior.
2. Two-layer G-Buffer/depth output and an explicit previous-depth/history reset
   policy, since current second-layer generation consults prior-frame depth.
3. Camera-space Z copy/pyramid with mip sampling and render-target transitions.
4. Portable pass inputs for projection/reconstruction and controls corresponding
   to AO radius, bias, intensity, sample count, and second-layer contribution.

**Observable acceptance**

- A deterministic scene containing nearby surfaces and an overlapping
  foreground/background pair produces valid first/second depth layers at the
  overlap; clearing/restarting history does not produce undefined output.
- With AO disabled and enabled for the same camera/frame, the AO debug view and
  shaded result differ in local contact/near-occlusion regions, not uniformly
  across the frame.
- Enabling second-layer contribution changes the AO result where the second
  layer contributes; disabling it uses the first layer only.
- The bilateral blur reduces sample noise without visibly bleeding across a
  strong camera-space depth discontinuity.
- Empty/background pixels and resized targets remain finite and correctly
  initialized. The test records deterministic captures or pixel checks so later
  MSL changes can be compared; no numerical parity threshold with D3D11 is
  claimed until a reference scene/capture is agreed.

## HLSL and MSL migration constraints

- The principal legacy shaders are HLSL in `Engine/Shaders`. `ShaderManager`
  creates vertex, geometry, and pixel shader objects; the two-layer G-Buffer
  generator uses all three stages. The files named `ComputeAO` and
  `ComputeRadiosity` are **pixel shaders**, not compute shaders. No HLSL compute
  shader entry point was found in the engine shader set.
- G-Buffer generation writes five `SV_Target` MRT outputs and routes geometry
  to two array slices. Other shaders read `Texture2DArray`/`Texture2D`,
  sample mip chains, issue `Load`, use sampler states, and bind constant buffers
  and texture resources through explicit D3D registers (for example `t0`–`t11`,
  `s0`–`s10`, and `b0`–`b6`). See [G-Buffer globals](../Engine/Shaders/DeepGBuffer_Gen_Globals.hlsl),
  [lighting constants](../Engine/Shaders/LightingConstants.hlsl), and
  [GI constants](../Engine/Shaders/GIHelpers.hlsl).
- The shader family includes vertex (`.vs.hlsl`), geometry (`.gs.hlsl`), and
  pixel (`.ps.hlsl`) programs, with fullscreen post passes using a fullscreen
  vertex shader plus pixel shaders. Metal has no direct D3D-style geometry
  stage equivalent in this design; the two-layer generation algorithm therefore
  needs an explicit implementation choice (for example, a different draw/pass
  strategy), not a mechanical source conversion.
- Matrix/depth conventions, MRT formats, texture-array interpretation, sampler
  behavior, constant packing, synchronization, and temporal history are all
  observable constraints. The current Metal slice has a verified direct-MSL
  vertex/fragment build and package path; no local HLSL-to-MSL or
  HLSL-to-SPIR-V conversion path is available. The [shader workflow
  evaluation](shader-workflow-evaluation.md) records tool availability,
  representative shader findings, compiler diagnostics, and the provisional
  recommendation. Keep conversion unselected until representative shader
  output and semantics are validated.

## Windows project and D3D11 retirement impact

| Project | Current relationship | Retirement consequence / disposition |
| --- | --- | --- |
| [`Engine.sln`](../Engine.sln) and [`Engine/Build/Engine.vcxproj`](../Engine/Build/Engine.vcxproj) | Visual Studio solution/project for the D3D11 static engine; the engine project links `d3d11`, `dxgi`, `pdh`, `winmm`, `dinput8`, and `dxguid`. | Removing the legacy engine project breaks existing Windows consumers until they are migrated or deliberately retired. Keep it as a reference for now; do not add it to macOS CMake. |
| [`Win32 App`](../Win32%20App/Win32%20App.vcxproj) | Desktop Win32 executable links Engine and Utils plus D3D11/DXGI. It owns a `WinMain`/Win32 window and message loop. | D3D11 engine retirement removes its current renderer dependency; the Win32 host itself is not a macOS host migration target. **Drop/defer** unless continued Windows desktop support is required. |
| [`Engine_WinRT`](../Engine_WinRT/Engine_WinRT.vcxproj) and [`WinRT App`](../WinRT%20App/WinRT%20App.vcxproj) | C++/CX/XAML engine wrapper links Engine/Utils and D3D11/DXGI; the XAML app uses D3D11, Direct2D/DirectWrite and WinRT UI/property-panel code. The app project references `Engine_WinRT`. | Removing Engine breaks this wrapper/app, and their UI is not portable core. Retain desired property/editor behavior as requirements, not as a commitment to port C++/CX/XAML or Direct2D/DirectWrite. **Defer** pending an explicit host/editor requirement. |
| [`Utils`](../Utils/Utils.vcxproj) | Shared static utility project used by Engine and apps; includes DirectXMath-based math and loader/data helpers, plus other utilities. | Do not delete wholesale with Engine: inventory which math, loader, threading, and string helpers are still required and split/replace those that are. DirectXMath leakage is a migration blocker for portable public data. **Adapt selectively**. |
| [`AssetConverter`](../AssetConverter/AssetConverter.vcxproj) | Separate Windows tool depending on Utils, Assimp, DirectXTex, Shlwapi, and Ole32; not a Metal target. | Retiring D3D11 does not by itself settle the converter's need: identify the accepted asset formats and replacement before dropping imported data capability. **Defer** and assess separately. |

D3D11 retirement therefore affects more than the renderer source: current
Win32/WinRT applications, the C++/CX wrapper, shared Utils, and the Windows
asset converter have independent dependency or behavior questions. Excluding
legacy files from the new CMake target is not equivalent to retiring them.
Actual source/project deletion requires a separate reviewed change after
required data and behavior have dispositions.

## Future backends: separate evaluation

D3D12 and Vulkan are not port targets for this inventory and neither has a
current implementation requirement. After the Metal renderer and selected
features exist, evaluate each only against a concrete need. Review resource
lifetime, command submission, synchronization, shader stages, render-target
formats, and the costs of maintaining another backend. The renderer seam should
represent demonstrated engine-level work and should be revised deliberately;
do not recreate D3D11 APIs as generic wrappers or build a multi-backend framework
in anticipation of an unvalidated future.

## Open findings to resolve during implementation

- The exact portable scene/mesh/material data contract and binary-format
  compatibility requirements are not specified.
- The Metal strategy for two-layer G-Buffer generation must replace the legacy
  geometry-shader expansion and define deterministic first-frame/history reset
  behavior.
- Direct MSL is the provisional choice for Metal bring-up; HLSL conversion
  remains unvalidated, and the geometry-stage algorithm still needs a Metal
  redesign rather than source conversion.
- The old `.vs`/`.ps` shader and font/text paths have unclear live status.
- There is no validated reference capture or agreed numeric image tolerance for
  D3D11-to-Metal feature parity.
- Continued Windows desktop/WinRT support and the future asset converter are
  product decisions; this inventory does not infer that they must be ported or
  deleted.
