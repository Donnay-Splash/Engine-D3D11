# Metal Shader Converter

This directory vendors Apple's `libmetalirconverter` and C API headers, version
4.0.1. It does not include Apple's standalone `metal-shaderconverter` executable.

The universal macOS library was obtained from the `v2026-08-27` release of
[shader-compilers-bin](https://github.com/double-buffer/shader-compilers-bin/releases/tag/v2026-08-27),
which packages Apple's Metal Shader Converter 4.0 beta. The downloaded archive
was `macos_MetalShaderConverter_4_0_beta_arm64.zip` (SHA-256:
`e5854105c1065ca73f1041a9d7dc4347fbeb52e45782a87cdf45161357d0fe69`).
The vendored `libmetalirconverter.dylib` SHA-256 is
`1af7a62b73330e56e7e47ad7cf3582b3b79b5cdee1c8fef9ab1387fb15b71d2e`.

Apple's [Metal Shader Converter download page](https://developer.apple.com/downloads?q=Shader%20Converter)
provides the official package and current license. Apple's Metal Shader
Converter agreement permits installing, using, testing, and distributing the
Dynamic Libraries solely for shader conversion. Use of the library remains
subject to Apple's agreement and supported Apple hardware/software
requirements. The vendored API headers include their Apache-2.0 license and
acknowledgements.
