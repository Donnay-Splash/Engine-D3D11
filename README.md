D3D11 rendering framework used for my dissertation.
The dissertation is an investigation of the applications of deep G-Buffers for real-time screen-space rendering.

Deep G-Buffers is a 2-layer Layered Depth Image technique developed by Michael Mara, Morgan McGuire, Derek Nowrouzezahrai and David Luebke. The latest paper can be found [here](http://graphics.cs.williams.edu/papers/DeepGBuffer16)

Some GIFs below demonstrate some of the features included in the framework.

# Scene hierarchy editor
![Scene hierarchy editor](https://github.com/Donnay-Splash/Donnay-Splash.github.io/blob/master/Edit%20teapot.gif)

To be continued...

## macOS app setup

The CMake `engine-macos` target builds a native `.app` with a centered, resizable 800×600-point AppKit window and a MetalKit view. It does not compile or link the Windows/D3D11 engine or render a Metal frame. If a usable Metal device is unavailable, the app displays an error and exits. AppKit and MetalKit integration is confined to a small Objective-C++ host file; engine integration and rendering are later milestones in the [macOS Metal engine roadmap](docs/macos-metal-engine-roadmap.md).

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

The original Makefile still builds a standalone command-line bootstrap. It does not compile or link the engine or utilities, create a window, or render. The CMake target is now the native GUI app described above; it creates a window and `MTKView` but does not render a frame or use engine code. Neither path changes the Visual Studio projects.

To use the original Makefile path, build and run it from the repository root:

```sh
make -C macOS
./macOS/build/engine-macos
```

The Makefile build output is ignored by Git. A successful bootstrap build verifies only that this standalone executable compiles and runs on macOS; it is not evidence that the engine links or that rendering works there.

## Work still required to run the engine on macOS

The current CMake app is a native window and `MTKView` host, but it does not display engine-rendered output. The existing D3D11 engine cannot simply be linked unchanged. At minimum, completing the engine port entails:

1. Isolating the Windows application host, Windows APIs, and platform-specific timing/input facilities from the engine code intended for macOS.
2. Defining a thin graphics API interface and implementing a Metal renderer behind it. Metal is the initial target; the existing D3D11 code is migration reference to selectively preserve from, not a promised active Windows backend.
3. Replacing DirectXMath dependencies in shared engine-facing types with a platform-independent math layer, or another implementation that can be used by both platform targets.
4. Adding a CMake target for the portable engine components and connecting them to the macOS host, then validating the engine target independently of Visual Studio project files.

None of those engine, math, or renderer ports are part of the Feature 2 host.
