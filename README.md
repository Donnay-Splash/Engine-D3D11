D3D11 rendering framework used for my dissertation.
The dissertation is an investigation of the applications of deep G-Buffers for real-time screen-space rendering.

Deep G-Buffers is a 2-layer Layered Depth Image technique developed by Michael Mara, Morgan McGuire, Derek Nowrouzezahrai and David Luebke. The latest paper can be found [here](http://graphics.cs.williams.edu/papers/DeepGBuffer16)

Some GIFs below demonstrate some of the features included in the framework.

# Scene hierarchy editor
![Scene hierarchy editor](https://github.com/Donnay-Splash/Donnay-Splash.github.io/blob/master/Edit%20teapot.gif)

To be continued...

## macOS app setup

The CMake `engine-macos` target builds a native `.app` with an AppKit window and a Metal presentation surface. The portable C++ engine builds as the static `engine` library. The Metal backend is implemented in C++ with Apple's vendored metal-cpp headers; Objective-C++ remains in the macOS frontend for AppKit integration. Metal shaders are native MSL compiled to a bundled metallib. The backend enumerates Metal adapters and creates the device used by Engine. Hardware-backed Metal integration tests are available on macOS with a Metal-capable device. The architecture and migration are described in the [macOS Metal engine Technical Design](docs/macos-metal-engine-tdd.md) and the [graphics-core abstraction proposal](docs/graphics-core-abstraction-proposal.md).

Install Xcode Command Line Tools or Xcode, CMake, and Make. The Apple SDK supplies AppKit, Metal, and MetalKit as system frameworks. Verify the toolchain with:

```sh
xcode-select -p
xcodebuild -version
xcrun --sdk macosx --show-sdk-path
clang --version
cmake --version
make --version
xcrun --find metal
xcrun --find metallib
```

Configure with the Xcode generator, then build and open the project from the repository root:

```sh
cmake -S . -B build/macos -G Xcode \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=26
cmake --build build/macos --target engine-macos engine-metal-integration-tests
open build/macos/EngineMacOS.xcodeproj
```

Run the app from the generated Xcode development scheme to enable Metal API Validation before Metal device creation. The backend cannot query or enable API Validation at runtime. Running the built app directly (for example with `open build/macos/engine-macos.app`) does not apply the Xcode scheme diagnostic setting.

Run the app-to-engine lifecycle tests with:

```sh
ctest --test-dir build/macos --output-on-failure
```

The portable engine library, math/data behavior tests, and lifecycle tests can also be built directly without building the macOS GUI target:

```sh
cmake -S . -B build/portable -DBUILD_TESTING=ON
cmake --build build/portable --target engine engine-lifecycle-tests engine-draw-tests portable-core-tests graphics-api-tests
ctest --test-dir build/portable --output-on-failure
```

The architecture and minimum deployment target default to arm64 and macOS 26 in CMake and can be overridden for another supported setup. In VS Code, **Build macOS app** configures and builds the `engine-macos` target.

## Build status and platform dependencies

The original projects use Visual Studio/MSBuild (`Engine.sln` and `.vcxproj` files), Visual C++ toolset v142, and the Windows 10 SDK. They use precompiled headers and do not declare an explicit C++ language standard in the project files. The solution contains Win32 and x64 projects as well as WinRT projects (including ARM configurations); it has no existing macOS target.

The desktop application starts at `Win32 App/main.cpp` with `WinMain`, then creates a Win32 window and message loop through `Application` (`RegisterClassEx`, `CreateWindowEx`, `PeekMessage`, `TranslateMessage`, and `DispatchMessage`). Its precompiled header includes `Windows.h` and D3D headers. Separately, the engine itself is not platform-neutral: its headers and sources use D3D11/DXGI types and APIs; its precompiled header includes `d3d11_4.h`, `d3dcommon.h`, and `dxgi1_3.h`. The engine project links the renderer libraries `d3d11.lib` and `dxgi.lib`, as well as `pdh.lib`, `winmm.lib`, `dinput8.lib`, and `dxguid.lib`. The engine's timer and FPS classes also use Windows timing APIs (`QueryPerformanceCounter` and `timeGetTime`). The shared math API in `Utils/Math/Math.h` is based on DirectXMath types, so compiling the engine's common code also requires addressing that dependency.

These dependencies are distinct:

- **Windows host/platform:** `WinMain`, HWND/message handling, Windows headers and system APIs; other projects use WinRT/XAML or Windows-specific services.
- **D3D11 renderer:** D3D11/DXGI interfaces, device creation, D3D shader/resource code, HLSL shaders, and `d3d11.lib`/`dxgi.lib`. The WinRT UI also uses Direct2D/DirectWrite.
- **Other Windows libraries:** the engine also links `pdh.lib`, `winmm.lib`, `dinput8.lib`, and `dxguid.lib`; these are not replacements for the D3D renderer.
- **Shared math and asset tools:** DirectXMath is exposed through the utility math types. The AssetConverter uses DirectXTex and Windows libraries such as Shlwapi and Ole32. DirectXTK is listed as a NuGet dependency in the desktop projects.

## Engine migration direction

The current CMake app is a native window and `MTKView` host; rendering is not currently implemented. The existing D3D11 engine cannot simply be linked unchanged. The agreed direction is a portable C++ engine with backend-neutral graphics interfaces and isolated graphics backend libraries. The app owns the native window and event loop, supplies its render surface to the engine, and remains a thin host.

The portable core includes public math and CPU scene/mesh/material data. Engine initializes the graphics device and swapchain, records the UV-triangle draw, and the macOS host connects that path to the Metal backend and its MSL shader library. Portable tests exercise the engine with fake graphics objects; a hardware-backed Metal integration test also exercises adapter/device creation, buffer and texture resources, graphics and compute pipelines, command submission, synchronization, and buffer copies. The app's on-screen presentation path is implemented but should still be exercised on the target display with Metal API Validation enabled through its Xcode scheme. This work does not migrate legacy serialization, property/editor bindings, or a complete scene graph. Later stages will inventory the original engine features and migrate selected scene and rendering behavior, prioritizing dissertation-critical work without assuming one-to-one parity. DirectXMath and Windows services must be separated or replaced where needed. D3D11 will not be ported: its existing projects remain temporary migration reference and will be retired after a review of migrated behavior and Windows-project impact. D3D12 and Vulkan are future evaluation targets, not current implementation commitments.

See the [Technical Design](docs/macos-metal-engine-tdd.md) for the module structure, feature disposition, migration stages, and acceptance criteria.
