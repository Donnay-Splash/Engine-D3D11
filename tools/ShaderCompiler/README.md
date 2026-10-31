# shader-compiler

Tool that builds all engine shaders. HLSL in `engine/shaders` is compiled to DXIL with `dxc`, then
converted to a `.metallib` with the vendored Apple Metal Shader Converter library. CMake does not
build shaders automatically.

```sh
brew install dxc
cmake --build build/macos --target shader-compiler
shader_compiler="$(find build/macos -type f -name shader-compiler -perm -111 -print -quit)"
"$shader_compiler" --config tools/ShaderCompiler/shaders.cfg
```

Options: `--target dxil|metal|all` (default `all`) and `--dry-run` (print commands only).

Output (see `outputDirectory` in the config, default `build/shaders`):
`dxil/<Shader>_<entry>.dxil`, `metal/<Shader>_<entry>.metallib`, and the converter reflection JSON
used by the Metal backend to encode shader resources. The macOS app bundles `metal/` into
`Resources/shaders`; the Metal integration tests read it from the same directory.

`shaders.cfg` lists the DXC executable, shader model, directories and, per `[shader]`, the source
and its `entry = <vs|ps|cs> <name>` lines. Paths are relative to the config file. The Metal
Shader Converter C API and dynamic library are vendored under `third_party/metal-shader-converter`;
the shader compiler links them directly on macOS and writes the converter's reflection JSON next to
each metallib.

Requires `dxc` on `PATH` (or set in the config). Metal conversion is available on macOS.
