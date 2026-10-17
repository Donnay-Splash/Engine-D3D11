# macOS Metal Engine Roadmap

## Goal

Build and run the engine from a native macOS application, then display engine-rendered content in its window using Metal.

The first implementation target is **macOS + Metal**. The app will use **AppKit + MetalKit**, and the build will use **CMake** rather than requiring Visual Studio project files. The existing D3D11 implementation is legacy reference material: retire it as an active backend while selectively preserving useful scene, asset, and rendering behavior.

This is a staged migration, not a promise to port every feature. First establish a working macOS build, window, and Metal frame; then bring across portable engine concepts and selected rendering features in small vertical slices.

## Decisions and boundaries

| Topic | Decision |
|---|---|
| macOS host and view | Native AppKit window hosting an `MTKView` |
| Build | CMake targets for portable engine code and the macOS executable |
| First graphics API | Metal only |
| Existing D3D11 renderer | Retire as an active backend; consult it while extracting behavior, data, and algorithms |
| D3D12 and Vulkan | Not in scope for this roadmap |
| Renderer architecture | A thin graphics API interface; implement Metal first and keep backend-specific resource management inside its implementation |
| Windows runtime compatibility | Not a goal of the new macOS target; identify and explicitly accept Windows-project impact before removing legacy code |

The portable engine core should not depend on AppKit, Metal, D3D11, Windows headers, or a graphics API. Put a thin renderer interface between that core and the backend implementation so the engine can remain portable and other backends can be considered later. Implement only Metal for now. Keep the interface focused on engine-level rendering work; do not reproduce D3D11's device/immediate-context model or expose backend resource types through the core.

## Current state

- `macOS/main.cpp` and `macOS/Makefile` build a standalone command-line bootstrap. They do not compile the engine, create a window, link Apple frameworks, or render.
- The engine and utilities are Visual Studio/MSBuild projects with precompiled headers. The engine project contains absolute Windows include paths and links D3D11, DXGI, PDH, WinMM, DirectInput, and DXGUID libraries.
- D3D11 is spread across the renderer, engine-facing headers, and resource classes. `D3DClass` owns device/swap-chain setup, but textures, buffers, shaders, pipeline state, render targets, and rendering calls also use D3D11 interfaces and operations. Replacing `D3DClass` alone will not make the rest of the engine portable.
- `Engine::Engine` coordinates input, scene state, and a D3D11-specific rendering pipeline. Separate portable engine behavior from GPU resource ownership and frame submission rather than trying to compile this class unchanged.
- `Utils/Math/Math.h` and its implementation wrap DirectXMath. The engine also uses Windows timing APIs. These are independent portability blockers even after the renderer is replaced.
- Windows app hosts, WinRT/XAML UI, D3D11 resources, Direct2D/DirectWrite code, and the Windows-only AssetConverter are distinct from the macOS host and the portable engine core.
- Scene hierarchy, scene/component/property data, mesh concepts, asset formats, and the Deep G-Buffer algorithms are candidates for reuse. HLSL source and D3D11 resource wrappers are not directly usable by Metal; shader stages, data layout, and resource binding need deliberate review and porting.

## macOS build dependencies

### Required tools

- Apple Clang and the macOS SDK, normally provided by Xcode Command Line Tools or Xcode.
- CMake and Make/Ninja as the selected CMake generator requires.
- An explicitly selected macOS deployment target and supported architecture set.

Check the host before adding dependencies or selecting versions:

```sh
xcode-select -p
xcodebuild -version
xcrun --sdk macosx --show-sdk-path
xcrun --sdk macosx --show-sdk-version
clang --version
cmake --version
xcrun --find metal
xcrun --find metallib
```

AppKit, Metal, and MetalKit are frameworks in the Apple SDK; they are not separate packages to install. The Metal command-line compiler tools are supplied as the Xcode Metal Toolchain component and may need to be downloaded separately. Full Xcode may be useful for IDE debugging and app signing, but a command-line CMake build should not require opening an Xcode project. Do not add a package manager dependency for a native window or Metal view.

### Verified development workstation baseline

Checked on 2026-10-15:

- **Host:** Apple M1 MacBook Air, arm64, with a 7-core Apple GPU; macOS 26.6.1.
- **Apple tools:** Xcode 26.6 (build 17F113), Apple Clang 21.0.0, macOS SDK 26.5.
- **Build tools:** CMake 4.4.4 (installed with Homebrew for this work) and GNU Make 3.81 are available. Use CMake's `Unix Makefiles` generator; Ninja is not installed.
- **Metal tools:** Metal Toolchain component 17F109 is installed; both `metal` and `metallib` resolve through `xcrun`.
- **Frameworks:** AppKit, Metal, and MetalKit are present in the selected macOS SDK.
- **Compile smoke tests:** Objective-C++ imports of AppKit and MetalKit compile with the macOS 13.1 deployment target, and a minimal Metal kernel compiles with `xcrun metal`.
- **Initial target assumption:** use Apple Silicon (`arm64`) for first bring-up because that is the available development/test machine. Use macOS 13.1 as a provisional deployment floor, not a release-support commitment; the SDK supports older targets, but there is no broader test-device or user-support matrix yet. Revisit this before promising Intel support or a minimum OS to users, and avoid APIs newer than the selected deployment target unless availability is checked.

The first configure command, once the CMake project exists, should make those choices explicit:

```sh
cmake -S . -B build/macos -G "Unix Makefiles" \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=13.1
cmake --build build/macos
```

## What to preserve, replace, and defer

| Area | Initial disposition |
|---|---|
| Scene graph, scene nodes, components, and property model | Review for extraction into the portable core; retain behavior and data that do not intrinsically depend on D3D11 |
| Mesh and asset data | Preserve formats and CPU-side representations where practical; separate them from GPU buffers |
| Deep G-Buffer and screen-space rendering algorithms | Preserve as reference and migrate selectively after a simple Metal scene renders |
| `D3DClass`, swap chain, D3D11 resource wrappers, and immediate-context calls | Replace with a Metal renderer and Metal resource management behind the thin graphics API interface; do not wrap the D3D11 objects themselves |
| DirectXMath wrapper | Replace or isolate behind portable math value types; test matrix conventions, transforms, and shader-facing layout |
| HLSL shaders | Preserve algorithm intent and source; evaluate HLSL authoring with cross-compilation to MSL and, if useful later, SPIR-V. Keep the initial runtime Metal-only and validate toolchain support before committing to a shader workflow |
| Geometry-shader and D3D11-specific pipeline behavior | Identify separately; redesign for Metal rather than assuming a one-to-one API mapping |
| Windows timers, input, file access, and diagnostics | Define only the platform-service seams needed by the macOS path, then provide macOS implementations |
| D3D11 font/text rendering, Direct2D/DirectWrite UI, Win32/WinRT hosts | May be abandoned; they are not prerequisites for the macOS engine-rendering goal |
| DirectXTex-based AssetConverter | Defer; document required asset formats and select a portable conversion path only when needed |
| D3D12 and Vulkan backends | Out of scope; reconsider architecture if these become explicit goals |

Do not delete legacy source as a shortcut to a clean macOS build. Keep it available while mapping reusable behavior, but do not require it to build as part of the new CMake macOS targets. Before any eventual removal, record which existing Windows apps/projects will stop building and confirm that loss of that runtime is accepted.

## Phased features and bite-sized tasks

### Feature 0 - Confirm the macOS build baseline (Complete)

1. **Record the host toolchain.** Capture available Apple Clang, CMake, Xcode Command Line Tools/Xcode, SDK version, Metal Toolchain, and `xcrun` availability. Current verified baseline: Apple Silicon M1/arm64, macOS 26.6.1, Xcode 26.6, SDK 26.5, CMake 4.4.4, GNU Make 3.81, and Metal Toolchain 17F109.
2. **Choose supported hardware and minimum OS.** For first bring-up, target arm64 and provisionally set macOS 13.1 as the deployment floor. This reflects the available Apple Silicon test machine, not a promise of release support; revisit if additional test machines or Intel support are required.
3. **Verify framework availability.** Confirm AppKit, Metal, and MetalKit resolve from the selected SDK; document that these come with the SDK, not a third-party dependency.
4. **Write down clean build commands.** Use CMake's `Unix Makefiles` generator and the explicit arm64/macOS 13.1 settings above; document run commands once the app target exists.

**Acceptance:** A new contributor can verify the required tools and SDK using documented commands, with no unrecorded dependency installation.

**Status:** Complete. The toolchain versions, SDK frameworks, Metal tools, architecture, and deployment floor have been checked on the documented Apple Silicon host. The CMake configure/build/run commands are now documented in the README.

### Feature 1 - Create a CMake build without Visual Studio dependencies

1. **Add the top-level CMake project.** Set the C++ standard explicitly and declare supported deployment target/architectures in a clear, overridable way.
2. **Create a minimal macOS executable target.** Initially compile only the existing bootstrap/host entry point; do not pull in legacy engine source lists wholesale.
3. **Avoid Objective-C++ where possible.** Use Metal-CPP for Metal APIs where relevant. AppKit and MetalKit host APIs require a small Objective-C++ boundary, which is implemented in Feature 2; keep other code in C++.
4. **Keep or retire the Make bootstrap deliberately.** Keep the Makefile as a separate command-line bootstrap alongside the CMake GUI target.

**Acceptance:** The documented CMake configure/build succeeds from a clean checkout and does not use `.sln` or `.vcxproj` files.

**Status:** Complete. A clean CMake configure/build/run succeeds without Visual Studio project files. The portable-core target is tracked in Feature 4, and Apple framework linkage is added to the Feature 2 host that needs it. The Makefile remains a separate command-line bootstrap.

### Feature 2 - Display an empty native macOS window

1. **Add the AppKit application lifecycle.** Start an `NSApplication`, own the application delegate, and terminate cleanly when the window closes.
2. **Create a resizable `NSWindow`.** Use the title "Engine" and an initial 800×600-point content size; center the window and handle resize without engine rendering.
3. **Host a MetalKit view.** Add an `MTKView` as the window content view. Select the system default Metal device to initialize it; if unavailable, show an actionable native alert and terminate with failure rather than displaying a blank fallback.
4. **Keep the host boundary narrow.** Use a small Objective-C++ file for AppKit/MetalKit lifecycle and view setup. Keep other code in C++; use Metal-CPP for Metal APIs where relevant.
5. **Package and exercise the app.** Build a `.app` bundle without adding code signing unless the local run/debug workflow requires it. Verify launch, resize, close, and repeat launch.

**Acceptance:** Running the CMake-built app opens a native, resizable window and exits cleanly. The host has no dependency on the D3D11 engine.

**Status:** Complete. The CMake `.app` configures and builds, and the window was launched, resized, closed, and relaunched on the documented Apple Silicon host. The no-device error path is explicit but was not runtime-exercised because this host has an Apple M1 Metal device; macOS 13.1 remains a provisional deployment floor and was not runtime-tested.

### Feature 3 - Present a minimal Metal frame

1. **Reuse the host's Metal device.** Feature 2 selects the system default device for `MTKView`; report any later initialization failures as actionable errors.
2. **Create the command queue and view delegate.** Tie one frame's work to the `MTKView` draw lifecycle and avoid rendering when there is no drawable.
3. **Present a clear frame and primitive.** Configure drawable sizing and color format, clear the render target, submit the command buffer, and let the view present the drawable.
4. **Handle resize and shutdown.** Verify drawable size changes, view/window teardown, and repeated launch/close.
5. **Add a first shader pipeline.** Prove shader compilation, pipeline creation, command encoding, and presentation end to end with a minimal visible primitive. Use MSL directly for the first proof unless the HLSL cross-compilation investigation is already validated.

**Acceptance:** The window repeatedly displays a Metal-rendered frame and primitive, and errors in device, shader, or pipeline creation are visible rather than silently falling back to a blank success state.

**Status:** The CMake app builds and packages a native MSL triangle shader; the host creates a Metal pipeline and submits frames through the `MTKView` draw lifecycle. Shader rebuilds update the bundled library. An offscreen Metal check loaded the packaged shader, rendered the triangle, and confirmed the center pixel differs from the clear background. The app has also been visually confirmed working on Apple Silicon, and an initial native-app launch/resize/close/relaunch smoke check succeeded.

### Feature 4 - Extract a portable engine core

1. **Make a reuse inventory.** For engine headers/sources, classify each item as portable now, portable after adaptation, Metal-specific replacement, Windows-only, or deferred. Start with the scene, component, property, mesh, loader, and resource headers.
2. **Separate scene data from renderer ownership.** Keep scene hierarchy and CPU-side object/mesh descriptions independent from `ID3D11*` and Metal objects.
3. **Replace DirectXMath dependence.** Choose a portable math implementation and migrate the engine-facing vector, matrix, quaternion, and color types in small groups.
4. **Lock down math conventions.** Add tests for transforms, projection, multiplication order, coordinate handedness, matrix storage, and CPU-to-shader data before porting more shaders.
5. **Separate platform services.** Identify the minimum clock, input, file, and diagnostic operations needed by the core; ensure their public interfaces do not expose Windows types.
6. **Compile and test the core on macOS.** Add focused tests for math and the first portable scene/data types; prevent Windows and renderer headers from entering the core target.

**Acceptance:** The portable-core target builds with Apple Clang without Windows, D3D11, DXGI, DirectXMath, or Visual Studio headers/libraries; migrated math and scene behavior is covered by tests.

### Feature 5 - Render engine-owned content

1. **Define the first renderer-facing data contract.** Specify CPU-side vertex/index/material data and the minimum backend-independent renderer operations needed for one object.
2. **Create Metal-owned buffers and pipeline state.** Keep Metal resource lifetime and command encoding inside the Metal implementation, not in the scene model.
3. **Feed a mesh from the engine model.** Render a simple mesh selected or constructed through the engine scene/mesh path rather than hard-coding all geometry in the window host.
4. **Add camera and transform data.** Use the migrated math types and verify the GPU-facing matrix layout against the math tests.
5. **Choose shader build handling.** Integrate shader validation/compilation with CMake and select a deliberate development/deployment strategy for source and compiled libraries. Evaluate an HLSL authoring path that cross-compiles to MSL, and SPIR-V if a future backend needs it; measure language/stage coverage and surface compile/load errors.

**Acceptance:** A scene object represented by the engine's portable model is visibly rendered through Metal in the macOS window.

### Feature 6 - Migrate legacy rendering features selectively

1. **Establish a rendering feature inventory.** Map each existing shader/pass to its algorithm, inputs/outputs, D3D11-specific assumptions, estimated prerequisite capabilities, and a keep/port/defer decision.
2. **Port basic mesh shading first.** Establish vertex formats, transforms, material values, texture sampling, and depth behavior before migrating multi-pass effects.
3. **Evaluate geometry-shader replacements.** Identify HLSL geometry-shader use and redesign those passes using Metal-supported stages or other data-generation approaches.
4. **Prototype the HLSL cross-compilation workflow.** Compare available compiler/conversion paths for HLSL-to-MSL and optional SPIR-V output; verify shader semantics, stage support, diagnostics, and CMake integration before adopting it.
5. **Port compute and render-target capabilities as needed.** For each selected feature, define texture formats, synchronization/encoder boundaries, and intermediate outputs before implementation.
6. **Reassess Deep G-Buffer work.** Decide which two-layer G-Buffer, temporal, radiosity, AO, and post-processing features are worth porting after the base scene path is stable; do not promise feature parity by default.
7. **Abandon or separately scope legacy UI and tools.** Font/text rendering, Direct2D/DirectWrite UI, Win32/WinRT hosts, and the AssetConverter can be dropped unless a concrete macOS requirement justifies a replacement.

**Acceptance:** Every migrated effect has a specific visible or testable result. Unported behavior is recorded as deferred rather than implied to work because the app can display a frame.

### Feature 7 - Retire D3D11 source intentionally

1. **Track replacements against the reuse inventory.** Mark each legacy scene, asset, shader, and rendering capability as migrated, intentionally dropped, or still needed as reference.
2. **Identify Windows impact.** List the Win32 and WinRT projects that depend on D3D11 code and state plainly which will no longer build if that code is removed.
3. **Confirm retirement boundaries.** Separate excluding old code from the macOS CMake targets (early and required) from deleting/archiving source or changing Windows projects (later and explicit).
4. **Remove only reviewed code.** Delete or archive D3D11-specific files only when no longer needed as reference and when the loss of Windows support is accepted.

**Acceptance:** The macOS targets have no accidental D3D11/Windows dependency, and every removed capability has a recorded disposition.

## Validation strategy

- **Build checks:** configure/build the CMake targets from a clean checkout using the supported SDK, architecture, and deployment target.
- **Platform-boundary checks:** keep a small, auditable source list for the portable core; ensure it cannot include Windows/D3D headers or link Windows libraries.
- **Unit tests:** cover math conventions and portable scene/mesh data before those types are used to populate Metal buffers.
- **Runtime checks:** verify app launch/close, resize, Metal device creation, drawable availability, clear pass, shader/pipeline creation, primitive presentation, and engine-owned mesh presentation.
- **Migration checks:** validate each selected shader/effect independently and compare representative output/behavior to the legacy implementation where meaningful.
- **Failure checks:** report unavailable tools/SDK, missing device, shader compilation failure, and resource/pipeline creation errors explicitly.

## Risks and open questions for implementation

- **Renderer coupling is broad.** A thin graphics API interface is useful only if it describes engine-level work without leaking backend handles or reproducing D3D11's immediate-context assumptions. D3D11 resource ownership, state, shader binding, pass ordering, and swap-chain assumptions still need deliberate redesign inside the backend.
- **Math changes affect rendering.** DirectXMath and HLSL conventions can encode matrix layout, multiplication order, handedness, and depth-range assumptions. Validate these before attempting large shader ports.
- **Shader cross-compilation may be lossy.** HLSL-to-MSL or HLSL-to-SPIR-V conversion does not guarantee equivalent behavior for every shader stage or resource pattern. Prototype representative shaders and document limitations before making HLSL the required authoring language.
- **The feature endpoint needs agreement.** This roadmap's minimum visual proof is an engine-owned mesh rendered by Metal. Reproducing the dissertation's full Deep G-Buffer feature set is a later, separately prioritized choice.
- **Existing documentation direction:** the README currently describes retaining D3D11 for Windows as a possible migration approach. This roadmap records the selected direction to retire it; update that older guidance when implementation begins so users are not left with conflicting project policy.
