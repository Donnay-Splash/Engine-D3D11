# Standalone shader build tool; not linked into the engine. Shaders are built by running it.
add_executable(shader-compiler
    main.cpp
    MetalShaderConverter.cpp
    MetalShaderConverter.h
    ShaderCompilerConfig.cpp
    ShaderCompilerConfig.h
)
target_compile_options(shader-compiler PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall>
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wextra>
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Werror>
)

if(APPLE)
    set(METAL_SHADER_CONVERTER_ROOT "${PROJECT_SOURCE_DIR}/third_party/metal-shader-converter")
    set(METAL_SHADER_CONVERTER_LIBRARY
        "${METAL_SHADER_CONVERTER_ROOT}/lib/libmetalirconverter.dylib")
    if(NOT EXISTS "${METAL_SHADER_CONVERTER_LIBRARY}")
        message(FATAL_ERROR "The vendored Metal Shader Converter library is missing.")
    endif()

    target_include_directories(shader-compiler PRIVATE
        "${METAL_SHADER_CONVERTER_ROOT}/include"
    )
    target_link_libraries(shader-compiler PRIVATE "${METAL_SHADER_CONVERTER_LIBRARY}")
    set_target_properties(shader-compiler PROPERTIES
        BUILD_RPATH "${METAL_SHADER_CONVERTER_ROOT}/lib"
    )
endif()
