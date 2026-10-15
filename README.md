D3D11 rendering framework used for my dissertation.
The dissertation is an investigation of the applications of deep G-Buffers for real-time screen-space rendering.

Deep G-Buffers is a 2-layer Layered Depth Image technique developed by Michael Mara, Morgan McGuire, Derek Nowrouzezahrai and David Luebke. The latest paper can be found [here](http://graphics.cs.williams.edu/papers/DeepGBuffer16)

Some GIFs below demonstrate some of the features included in the framework.

# Scene hierarchy editor
![Scene hierarchy editor](https://github.com/Donnay-Splash/Donnay-Splash.github.io/blob/master/Edit%20teapot.gif)

To be continued...

## Build status and platform dependencies

The original projects use Visual Studio/MSBuild (`Engine.sln` and `.vcxproj` files), Visual C++ toolset v142, and the Windows 10 SDK. They use precompiled headers and do not declare an explicit C++ language standard in the project files. The solution contains Win32 and x64 projects as well as WinRT projects (including ARM configurations); it has no existing macOS target.

The desktop application starts at `Win32 App/main.cpp` with `WinMain`, then creates a Win32 window and message loop through `Application` (`RegisterClassEx`, `CreateWindowEx`, `PeekMessage`, `TranslateMessage`, and `DispatchMessage`). Its precompiled header includes `Windows.h` and D3D headers. Separately, the engine itself is not platform-neutral: its headers and sources use D3D11/DXGI types and APIs; its precompiled header includes `d3d11_4.h`, `d3dcommon.h`, and `dxgi1_3.h`. The engine project links the renderer libraries `d3d11.lib` and `dxgi.lib`, as well as `pdh.lib`, `winmm.lib`, `dinput8.lib`, and `dxguid.lib`. The engine's timer and FPS classes also use Windows timing APIs (`QueryPerformanceCounter` and `timeGetTime`). The shared math API in `Utils/Math/Math.h` is based on DirectXMath types, so compiling the engine's common code also requires addressing that dependency.

These dependencies are distinct:

- **Windows host/platform:** `WinMain`, HWND/message handling, Windows headers and system APIs; other projects use WinRT/XAML or Windows-specific services.
- **D3D11 renderer:** D3D11/DXGI interfaces, device creation, D3D shader/resource code, HLSL shaders, and `d3d11.lib`/`dxgi.lib`. The WinRT UI also uses Direct2D/DirectWrite.
- **Other Windows libraries:** the engine also links `pdh.lib`, `winmm.lib`, `dinput8.lib`, and `dxguid.lib`; these are not replacements for the D3D renderer.
- **Shared math and asset tools:** DirectXMath is exposed through the utility math types. The AssetConverter uses DirectXTex and Windows libraries such as Shlwapi and Ole32. DirectXTK is listed as a NuGet dependency in the desktop projects.

## macOS command-line bootstrap

The first macOS target is deliberately a standalone command-line executable. It establishes a native Clang/Make build and entry point to build from, but does not compile or link the engine or utilities and does not create a window or render anything. It does not change the Visual Studio projects.

With Apple Clang and Make available (for example, from the Xcode Command Line Tools), build and run it from the repository root:

```sh
make -C macOS
./macOS/build/engine-macos
```

The build output is ignored by Git. A successful bootstrap build verifies only that this standalone executable compiles and runs on macOS; it is not evidence that the engine links or that rendering works there.

## Work still required to run the engine on macOS

The current macOS target is only a command-line bootstrap. Getting to a native app that displays engine-rendered output requires a macOS host and a portable engine core; the existing D3D11 engine cannot simply be linked unchanged. At minimum, this entails:

1. Isolating the Windows application host, Windows APIs, and platform-specific timing/input facilities from the engine code intended for macOS.
2. Defining a thin graphics API interface and implementing a Metal renderer behind it. Metal is the initial target; the existing D3D11 code is migration reference to selectively preserve from, not a promised active Windows backend.
3. Replacing DirectXMath dependencies in shared engine-facing types with a platform-independent math layer, or another implementation that can be used by both platform targets.
4. Adding a CMake build for the macOS host and portable engine components, then validating the engine target independently of Visual Studio project files.

None of those engine, math, or renderer ports are part of the bootstrap target.
