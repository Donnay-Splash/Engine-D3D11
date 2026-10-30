# shader-compiler

Command line tool that builds all engine shaders. HLSL in `engine/shaders` is compiled to DXIL with
`dxc`, then converted to a `.metallib` with Apple's `metal-shaderconverter`. CMake does not build shaders.

```sh
cmake --build build/macos --target shader-compiler
build/macos/tools/ShaderCompiler/shader-compiler --config tools/ShaderCompiler/shaders.cfg
```

Options: `--target dxil|metal|all` (default `all`) and `--dry-run` (print commands only).

Output (see `outputDirectory` in the config, default `build/shaders`):
`dxil/<Shader>_<entry>.dxil`, `metal/<Shader>_<entry>.metallib`, and the converter reflection JSON
used by the Metal backend to encode shader resources. The macOS app bundles `metal/` into
`Resources/shaders`; the Metal integration tests read it from the same directory.

`shaders.cfg` lists the tool paths, shader model, directories and, per `[shader]`, the source and its
`entry = <vs|ps|cs> <name> [extra metal-shaderconverter args]` lines. Paths are relative to the config file.

Requires `dxc` and `metal-shaderconverter` on `PATH` (or set in the config).
