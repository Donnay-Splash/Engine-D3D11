# Copilot Instructions

This repository is a hobby renderer used for educational purposes. It consists of platform specific front ends and back ends with a generic middle layer we call the Engine. 

## General
- The Engine should always remain platform agnostic and should not contain any platform specific code. 
- The front end and back end are responsible for providing the engine with the necessary platform specific functionality.
- Backend implementation and platform integration code must use C++ (`.cpp`); keep Objective-C++ confined to the macOS front end when AppKit interop requires it.
- Put each class or cohesive component in its own focused header/source file; keep unrelated implementations out of monolithic files.
- The graphics components, resources, buffers, textures, shaders, etc... should all be built modularly so that we can easily iterate on graphics features.
- The codebase should target c++20 and use modern C++ features where appropriate.
- Write simple, well-documented code and add comments only when they add real value.
- Log any risk or assumption you could not verify (for example, behavior untestable on the current platform) as a GitHub issue with `gh` (see `docs/agents/issue-tracker.md`), labelled `needs-triage`, instead of only mentioning it in prose.

## Repository Layout

```text
AssetConverter/       Existing asset converter; leave its implementation independent.
backends/             Top level for all graphics backends.
  metal/              C++ Metal backend, surface integration, and native Metal shaders.
tools/ShaderCompiler/ Shader build command line tool and its config.
build/                Generated build output; do not place source files here.
docs/                 Project documentation.
engine/               Platform-neutral engine library.
  Core/               Portable math and scene data.
  shaders/            HLSL shader sources.
  tests/              Engine lifecycle and portable-core tests.
macOS/                AppKit front end; links the engine and selected backend libraries.
.github/
  copilot-instructions.md
CMakeLists.txt        Defines the engine, backend, front-end, and test targets.
```

- Keep reusable, platform-neutral code in `engine/`; it builds as the static `engine` library and must not link graphics or platform frameworks.
- Put each graphics backend in its own `backends/<name>/` folder and static library. The current `backends/metal/` implementation builds as `engine-backend-metal`, uses vendored Apple metal-cpp, contains only C++ sources, and links to `engine`.
- Keep host/window/event-loop code in the matching front end. The macOS front end lives in `macOS/` and links the engine and the backend it uses.
- Add future platform front ends alongside `macOS/`; do not move their host code into `engine/`.

## Coding Standards
- Use camelCase for variable and function names, and PascalCase for class names, namespaces and file names.
- Use m_ for member variables and s_ for static variables and g_ for global variables.
- Use `#pragma once` for header files.
- Use `std::unique_ptr` and `std::shared_ptr` for memory management instead of raw pointers.
- Use `std::vector` and `std::array` for dynamic and fixed-size arrays, respectively.
- Use `std::string` for string manipulation instead of C-style strings.
- Use `const` and `constexpr` where appropriate to indicate immutability and compile-time evaluation.
- Don't rewrite shader code in c++ for testing purposes.
- Use `std::optional` for optional values instead of raw pointers or sentinel values.
- Use asserts to catch edge cases and invalid states at runtime.

## Tech Stack

### Runtime
- CMake for build system.
- We will support only modern graphics APIs, specifically Directx 12, Metal and Vulkan.
- Shader artifacts may be backend-specific. Shaders are authored only in HLSL under `engine/shaders` and built by `tools/ShaderCompiler` (DXC to DXIL, then the vendored Apple Metal Shader Converter library to metallib); CMake does not build shaders automatically.

### Testing
Use CTest to run tests. Keep portable engine logic testable without a graphics device; use fakes for renderer and platform lifecycle tests. Add backend integration tests only when the required graphics API is available. Metal API Validation is enabled through the generated Xcode development scheme and must be active before Metal device creation; the backend cannot query or enable it at runtime. Prefer Catch2 if a test framework becomes worthwhile; avoid creating a custom framework.

## Validation
When assisting inside a cloud-hosted environment, you must always run `.github/scripts/validate.sh` to remotely verify your Metal and CMake changes on Apple hardware before declaring a task complete. When assisting a user working on a local macOS machine, instruct them to run the same script for native, instant compilation.
