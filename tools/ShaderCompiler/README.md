# shader-compiler

Tool that builds all engine shaders. HLSL in `engine/shaders` is compiled to DXIL with `dxc`, then
converted to a `.metallib` with the vendored Apple Metal Shader Converter library. CMake does not
build shaders automatically.

```sh
cmake --build build/macos --target shader-compiler
shader_compiler="$(find build/macos -type f -name shader-compiler -perm -111 -print -quit)"
"$shader_compiler" --config tools/ShaderCompiler/ShaderCompiler.cfg
```

Options: `--target dxil|metal|all` (default `all`) and `--dry-run` (print commands only).

Output (see `outputDirectory` in the config, default `build/shaders`):
`dxil/<Shader>_<entry>.dxil`, `metal/<Shader>_<entry>.metallib`, and the converter reflection JSON
used by the Metal backend to encode shader resources. The macOS app bundles `metal/` into
`Resources/shaders`; the Metal integration tests read it from the same directory.

For every entry the Metal path also writes `metal/<Shader>_<entry>.reflection.json`, a
backend-neutral description (vertex inputs by semantic, resource bindings by type) that the engine
uses to build binding layouts and pipeline vertex layouts; its schema is documented in
`engine/Graphics/ShaderReflection.h`. Today it is derived from the Metal converter's reflection, so
`SRV` resources are reported as sampled textures and unnamed.

`ShaderCompiler.cfg` only parameterises the compiler: the DXC executable, shader model, output
directory, `includeDirectory` (where `<source>.hlsl` lives), the `programs` file and extra DXC
arguments. Paths are relative to the config file.

`engine/shaders/ShaderPrograms.cfg` is the single place that defines shaders. Each `[program]`
names `vertex`/`fragment` or `compute` stages as `<source>:<entry>`; `<source>` is the HLSL file name
without its extension. The compiler builds exactly the entries named there (nothing else is
compiled), and copies the file to `metal/ShaderPrograms.cfg` so the runtime reads the same config.
The Metal Shader Converter C API and dynamic library are vendored under
`third_party/metal-shader-converter`; the compiler links them directly on macOS and writes the
converter's reflection JSON next to each metallib.

Uses the vendored DXC in `third_party/dxc/macos` by default (`dxc` in the config; paths containing a
`/` are relative to the config file, bare names are looked up on `PATH`). Metal conversion is
available on macOS.

Builds are incremental: an entry is skipped when its outputs are newer than the HLSL source, the
config file, the DXC binary and every header it includes. Header dependencies are recorded by dxc
(`-MF`) in `dxil/<Shader>_<entry>.d` after each successful build. The VS Code "Launch macOS app"
configuration runs this via the "Build macOS shaders" task before building the app.

## Material parameter defaults

Material resources live in `space1`: one constant buffer of scalars/vectors plus sampled textures.
A trailing `// @default(...)` comment on the declaration supplies the default, which is written to
`*.reflection.json` and used by the engine's `MaterialLayout` for unset parameters:

```hlsl
cbuffer MaterialParams : register(b0, space1) {
    bool useTexture;   // @default(true)
    float4 tint;       // @default(1, 0.8, 0.4, 1)
};
Texture2D baseTexture : register(t0, space1); // @default(white)   (white, black or normal)
```

Arity must match the type (`bool` takes `true`/`false`/`0`/`1`, `int` a whole number, `floatN` N
numbers); a malformed annotation is a compile error. Unannotated parameters default to zero
(textures to white). Annotations on `space0` declarations are ignored.
