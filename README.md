D3D11 rendering framework used for my dissertation.
The dissertation is an investigation of the applications of deep G-Buffers for real-time screen-space rendering.

Deep G-Buffers is a 2-layer Layered Depth Image technique developed by Michael Mara, Morgan McGuire, Derek Nowrouzezahrai and David Luebke. The latest paper can be found [here](http://graphics.cs.williams.edu/papers/DeepGBuffer16)

Some GIFs below demonstrate some of the features included in the framework.

# Scene hierarchy editor
![Scene hierarchy editor](https://github.com/Donnay-Splash/Donnay-Splash.github.io/blob/master/Edit%20teapot.gif)

To be continued...

## macOS app setup

The CMake `engine-macos` target builds a native `.app` with a centered, resizable 800×600-point AppKit window and a MetalKit surface that continuously renders a simple MSL triangle over a clear background. The app links a static `engine` implementation library; the independently buildable `engine-core` contains the portable C++ lifecycle interface, row-major math, and CPU scene/mesh/material data. The AppKit host owns the window, surface, and event loop, while the engine library owns Metal device setup and rendering. If a usable Metal device or the shader/pipeline is unavailable, the app displays an error and exits. The architecture and migration are described in the [macOS Metal engine Technical Design](docs/macos-metal-engine-tdd.md).

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

Configure, build, and run from the repository root:

```sh
cmake -S . -B build/macos -G "Unix Makefiles" \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=13.1
cmake --build build/macos --target engine-macos
open build/macos/engine-macos.app
```

Run the app-to-engine lifecycle tests with:

```sh
ctest --test-dir build/macos --output-on-failure
```

The portable engine core, math/data behavior tests, and lifecycle tests can also be built directly without building the macOS GUI target:

```sh
cmake -S . -B build/portable -DBUILD_TESTING=ON
cmake --build build/portable --target engine-core engine-lifecycle-tests portable-core-tests
ctest --test-dir build/portable --output-on-failure
```

The architecture and deployment target default to arm64 and macOS 13.1 in CMake and can be overridden for another supported setup. The macOS 13.1 deployment floor remains provisional and has not been runtime-tested here. In VS Code, select **Launch macOS app** to configure and build the `engine-macos` target before launching it with LLDB. **Build macOS app** is also available as the default build task. The existing `make -C macOS` command remains available as a separate command-line bootstrap.

## Build status and platform dependencies

The original projects use Visual Studio/MSBuild (`Engine.sln` and `.vcxproj` files), Visual C++ toolset v142, and the Windows 10 SDK. They use precompiled headers and do not declare an explicit C++ language standard in the project files. The solution contains Win32 and x64 projects as well as WinRT projects (including ARM configurations); it has no existing macOS target.

The desktop application starts at `Win32 App/main.cpp` with `WinMain`, then creates a Win32 window and message loop through `Application` (`RegisterClassEx`, `CreateWindowEx`, `PeekMessage`, `TranslateMessage`, and `DispatchMessage`). Its precompiled header includes `Windows.h` and D3D headers. Separately, the engine itself is not platform-neutral: its headers and sources use D3D11/DXGI types and APIs; its precompiled header includes `d3d11_4.h`, `d3dcommon.h`, and `dxgi1_3.h`. The engine project links the renderer libraries `d3d11.lib` and `dxgi.lib`, as well as `pdh.lib`, `winmm.lib`, `dinput8.lib`, and `dxguid.lib`. The engine's timer and FPS classes also use Windows timing APIs (`QueryPerformanceCounter` and `timeGetTime`). The shared math API in `Utils/Math/Math.h` is based on DirectXMath types, so compiling the engine's common code also requires addressing that dependency.

These dependencies are distinct:

- **Windows host/platform:** `WinMain`, HWND/message handling, Windows headers and system APIs; other projects use WinRT/XAML or Windows-specific services.
- **D3D11 renderer:** D3D11/DXGI interfaces, device creation, D3D shader/resource code, HLSL shaders, and `d3d11.lib`/`dxgi.lib`. The WinRT UI also uses Direct2D/DirectWrite.
- **Other Windows libraries:** the engine also links `pdh.lib`, `winmm.lib`, `dinput8.lib`, and `dxguid.lib`; these are not replacements for the D3D renderer.
- **Shared math and asset tools:** DirectXMath is exposed through the utility math types. The AssetConverter uses DirectXTex and Windows libraries such as Shlwapi and Ole32. DirectXTK is listed as a NuGet dependency in the desktop projects.

## macOS command-line bootstrap

The original Makefile still builds a standalone command-line bootstrap. It does not compile or link the engine or utilities, create a window, or render. The CMake target is the native GUI app described above; its Metal triangle is rendered through the engine library. Neither path changes the Visual Studio projects.

To use the original Makefile path, build and run it from the repository root:

```sh
make -C macOS
./macOS/build/engine-macos
```

The Makefile build output is ignored by Git. A successful bootstrap build verifies only that this standalone executable compiles and runs on macOS; it is not evidence that the engine links or that rendering works there.

## Engine migration direction

The current CMake app is a native window and `MTKView` host. Its triangle is rendered through the engine library, but it does not yet render content from the portable scene/mesh data. The existing D3D11 engine cannot simply be linked unchanged. The agreed direction is a portable C++ engine core built as a static library, with a backend-neutral renderer seam and isolated macOS/Metal implementation. The app owns the native window and event loop, supplies its render surface to the engine, and remains a thin host; the engine owns graphics setup and rendering.

The first library-backed slice moved the existing triangle behind the engine interface. The portable-core slice adds public math and CPU scene/mesh/material data, but does not migrate legacy serialization, property/editor bindings, GPU layouts/resources, or a complete scene graph. Later stages will inventory the original engine features and migrate selected scene and rendering behavior, prioritizing dissertation-critical work without assuming one-to-one parity. DirectXMath and Windows services must be separated or replaced where needed. D3D11 will not be ported: its existing projects remain temporary migration reference and will be retired after a review of migrated behavior and Windows-project impact. D3D12 and Vulkan are future evaluation targets, not current implementation commitments.

See the [Technical Design](docs/macos-metal-engine-tdd.md) for the module structure, feature disposition, migration stages, and acceptance criteria.
