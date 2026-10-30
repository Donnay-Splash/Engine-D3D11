# Standalone shader build tool; not linked into the engine. Shaders are built by running it.
add_executable(shader-compiler main.cpp ShaderCompilerConfig.cpp ShaderCompilerConfig.h)
target_compile_options(shader-compiler PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall>
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wextra>
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Werror>
)
