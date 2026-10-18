# Shader workflow evaluation

## Decision

Author shaders in Metal Shading Language (MSL) for the initial Metal renderer,
and keep the existing HLSL as behavioral reference material. Do not add a
converter, dependency, cross-backend authoring format, or bulk shader rewrite
for this slice. The current MSL vertex/fragment pair compiles and is packaged
by the existing CMake build. No HLSL-to-MSL converter or complete
HLSL-to-SPIR-V-to-MSL toolchain is installed here, so conversion correctness
and runtime parity remain unproven.

This is a choice for Metal bring-up, not a claim that MSL is portable or that
conversion is impossible. Revisit the decision when a concrete second backend
or a representative feature migration justifies the additional toolchain.

## Repository shader evidence

The engine shader directory contains 4 `.vs.hlsl` vertex shaders, 15
`.ps.hlsl` pixel shaders, 2 `.gs.hlsl` geometry shaders, 13 shared HLSL
includes, and no `.cs.hlsl` files, `[numthreads]` attributes, or
`SV_DispatchThreadID` semantics. `DeepGBuffer_ComputeAO.ps.hlsl` and
`DeepGBuffer_ComputeRadiosity.ps.hlsl` are pixel shaders despite their names.
The existing macOS renderer's [`Triangle.metal`](../macOS/Triangle.metal)
contains the vertex entry point `triangle_vertex` and fragment entry point
`triangle_fragment`.

The representative legacy shader family needs more than syntax translation:

- [`DeepGBuffer_Gen.vs.hlsl`](../Engine/Shaders/DeepGBuffer_Gen.vs.hlsl)
  transforms row-vector values with `mul`, reads row-major matrices from
  constant buffers, and passes camera-space positions, normals, tangents,
  bitangents, and texture coordinates between stages.
- [`DeepGBuffer_Gen.ps.hlsl`](../Engine/Shaders/DeepGBuffer_Gen.ps.hlsl)
  samples textures and a texture array, can discard fragments, and writes five
  render targets (`SV_Target0` through `SV_Target4`).
  [`DeepGBuffer_Gen_Globals.hlsl`](../Engine/Shaders/DeepGBuffer_Gen_Globals.hlsl)
  assigns resources through D3D registers (`t0`–`t7`, `s0`/`s7`, and `b1`–`b3`).
  A new MSL interface must explicitly map these to Metal buffer, texture, and
  sampler indices and validate struct layout, matrix convention, formats, and
  stage interfaces; matching register numbers alone does not establish
  equivalent behavior.
- [`DeepGBuffer_Gen.gs.hlsl`](../Engine/Shaders/DeepGBuffer_Gen.gs.hlsl)
  uses `TriangleStream` to emit each triangle for two G-Buffer layers and
  selects the destination array layer per emitted vertex. Metal has no
  equivalent D3D geometry stage in this render pipeline. Preserve the result
  only through a deliberately redesigned draw/pass strategy; this stage is not
  a mechanical conversion target.

The lack of an HLSL compute shader means this ticket has no legacy compute
program to use as a compute-conversion sample. A future compute pass needs its
own input, resource, dispatch, synchronization, and output validation; the
`ComputeAO` filename is not evidence of compute-stage support.

## Workflow comparison

| Workflow | What it does | Evidence and constraints |
| --- | --- | --- |
| **Direct MSL (available and verified)** | Compile the renderer's MSL to AIR, link a Metal library, package it with the macOS app, and load it by function name. | `xcrun -sdk macosx metal -c macOS/Triangle.metal -o build/issue-4-cmake/Triangle.air` and `xcrun -sdk macosx metallib build/issue-4-cmake/Triangle.air -o build/issue-4-cmake/default.metallib` succeeded. The CMake app build generated the library and copied it to `engine-macos.app/Contents/Resources/default.metallib`. `MetalRenderer.mm` loads `default.metallib` from the main bundle, then looks up both entry-point names. This validates the existing simple vertex/fragment path, not parity for a legacy effect. |
| **Apple Metal Shader Converter (not available here)** | Apple documents [DXIL input producing a `.metallib`](https://developer.apple.com/metal/shader-converter/); a source-HLSL workflow would first need HLSL-to-DXIL compilation. | `xcrun --find metal-shaderconverter` reports that the utility is unavailable. Apple's [requirements](https://developer.apple.com/metal/shader-converter/) specify macOS 13/Xcode 15 and say converted libraries require Argument Buffers Tier 2 support on macOS 14 or later for the listed target platforms; the app's current deployment target is macOS 13.1, so compatibility would need deliberate validation. The [DXC README](https://github.com/microsoft/DirectXShaderCompiler/blob/main/README.md) says a DXC built with the converter available can also use `-metal`; neither component is available in this environment. |
| **DXC → SPIR-V → SPIRV-Cross → MSL (not available here)** | The [DXC README](https://github.com/microsoft/DirectXShaderCompiler/blob/main/README.md) documents HLSL-to-SPIR-V output; [SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross/blob/main/README.md) documents SPIR-V-to-MSL translation. | `dxc`, `spirv-cross`, and the SPIR-V validation/optimization utilities are not installed. This multi-step route has not been run against the repository shaders. DXC's [SPIR-V feature manual](https://github.com/microsoft/DirectXShaderCompiler/blob/main/docs/SPIR-V.rst) documents resource/register mapping and HLSL-to-SPIR-V rules; descriptor/binding remapping, entry interfaces, constant-buffer layout, and Metal-specific behavior would still need inspection and output validation. |
| **glslang → SPIR-V → SPIRV-Cross (not available and not recommended as a new basis)** | glslang has an HLSL frontend and a SPIR-V backend. | `glslang` and `glslangValidator` are not installed. Its [current README](https://github.com/KhronosGroup/glslang/blob/main/README.md) says the HLSL frontend is deprecated as of April 2026 and will be removed in the next major version; it characterizes that frontend as an approximation of HLSL. Do not select this path for new work without a specific reason to accept that maintenance risk. |

Metal's [function type API](https://developer.apple.com/documentation/metal/mtlfunctiontype)
has no D3D-style geometry function type. Apple's `metal` compiler is an MSL
compiler, not an HLSL converter. Passing
the representative `.vs.hlsl`, `.ps.hlsl`, and `.gs.hlsl` files to
`xcrun -sdk macosx metal -c` failed with diagnostics such as `unknown type name
'cbuffer'` (vertex/pixel) and `unknown type name 'float4'` (geometry). The
installed Apple Clang HLSL frontend probe also stopped before shader parsing
because its built-in `hlsl.h` header is unavailable. Neither failure is a
conversion result or evidence about whether a separate converter could handle
these programs.

## Verification record

Environment: macOS 26.6.1, Apple Metal compiler 32023.883, Apple Clang 21.0.0,
CMake 4.4.4. No package was installed.

The following checks ran in this worktree:

```sh
cmake -S . -B build/issue-4-cmake -G 'Unix Makefiles' \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=13.1
cmake --build build/issue-4-cmake
ctest --test-dir build/issue-4-cmake --output-on-failure
file build/issue-4-cmake/engine-macos.app/Contents/Resources/default.metallib

xcrun -sdk macosx metal -c macOS/Triangle.metal \
  -o build/issue-4-cmake/Triangle.air
xcrun -sdk macosx metallib build/issue-4-cmake/Triangle.air \
  -o build/issue-4-cmake/default.metallib

xcrun -sdk macosx metal -c Engine/Shaders/DeepGBuffer_Gen.vs.hlsl \
  -o build/issue-4-shader-check/legacy-vs.air
xcrun -sdk macosx metal -c Engine/Shaders/DeepGBuffer_Gen.ps.hlsl \
  -o build/issue-4-shader-check/legacy-ps.air
xcrun -sdk macosx metal -c Engine/Shaders/DeepGBuffer_Gen.gs.hlsl \
  -o build/issue-4-shader-check/legacy-gs.air

for tool in dxc glslangValidator spirv-cross spirv-val spirv-opt; do
  command -v "$tool" || printf '%s: not found\n' "$tool"
done
xcrun --find metal-shaderconverter

clang -x hlsl -fsyntax-only -I Engine/Shaders \
  -Xclang -hlsl-entry -Xclang VSMain \
  --target=dxil-pc-shadermodel6.0-vertex \
  Engine/Shaders/DeepGBuffer_Gen.vs.hlsl
```

The CMake build succeeded, produced a Metal library in the app bundle, and the
existing CTest lifecycle test passed (1/1). The three legacy HLSL compiler
invocations intentionally failed as described above. This was a compiler/build
check, not an app launch or image-comparison test; no visual parity or runtime
shader-load claim is made.

## What would justify another workflow

Before choosing a shared authoring format or conversion toolchain, compile
representative legacy vertex and pixel shaders (including the G-Buffer MRT and
resource cases) with the candidate tools, inspect diagnostics and generated
bindings, and validate GPU output against agreed deterministic captures.
Measure build integration, runtime packaging/loading, supported target OS and
GPU requirements, tool distribution/maintenance, and any changes to matrix,
depth, sampler, and resource semantics. Evaluate the two-layer geometry
algorithm as a separate redesign rather than using stage-conversion success as
a proxy. If a real compute feature is selected, include that exact compute
shader in its own validation. Only select a cross-backend path after it
demonstrates these requirements for each backend the project actually needs.

## Primary references

- Apple, [Metal Shader Converter](https://developer.apple.com/metal/shader-converter/) — DXIL input and Metal library output, requirements, and supported shader-model table.
- Apple, [`MTLFunctionType`](https://developer.apple.com/documentation/metal/mtlfunctiontype) — Metal shader function types; no D3D geometry-stage function type.
- Microsoft, [DirectX Shader Compiler README](https://github.com/microsoft/DirectXShaderCompiler/blob/main/README.md) — HLSL compiler and DXIL/SPIR-V targets; optional `-metal` integration when built with Apple's converter.
- Microsoft, [HLSL to SPIR-V Feature Mapping Manual](https://github.com/microsoft/DirectXShaderCompiler/blob/main/docs/SPIR-V.rst) — semantics, stages, and resource/register mappings for the Vulkan SPIR-V backend.
- Khronos, [SPIRV-Cross README](https://github.com/KhronosGroup/SPIRV-Cross/blob/main/README.md) — SPIR-V-to-MSL conversion and resource binding considerations.
- Khronos, [glslang README](https://github.com/KhronosGroup/glslang/blob/main/README.md) — current HLSL frontend approximation and deprecation notice.
