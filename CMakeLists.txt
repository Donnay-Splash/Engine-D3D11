cmake_minimum_required(VERSION 3.20)

set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "macOS architectures")
set(CMAKE_OSX_DEPLOYMENT_TARGET "13.1" CACHE STRING "Minimum macOS deployment target")

project(EngineMacOS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)

add_library(engine-core STATIC macOS/Engine/Engine.cpp)
target_include_directories(engine-core PUBLIC macOS/Engine)
target_compile_options(engine-core PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall>
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wextra>
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Werror>
)

if(BUILD_TESTING)
    add_subdirectory(macOS/tests)
endif()

if(APPLE)
    enable_language(OBJCXX)
    set(CMAKE_OBJCXX_STANDARD 11)
    set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)
    set(CMAKE_OBJCXX_EXTENSIONS OFF)

    find_library(APPKIT_FRAMEWORK AppKit REQUIRED)
    find_library(METAL_FRAMEWORK Metal REQUIRED)
    find_library(METALKIT_FRAMEWORK MetalKit REQUIRED)
    find_program(XCRUN_EXECUTABLE xcrun REQUIRED)

    add_library(engine STATIC
        macOS/Engine/MacEngine.mm
        macOS/Engine/MetalSurfaceAdapter.mm
        macOS/Engine/MetalRenderer.mm
    )
    target_include_directories(engine PRIVATE macOS/Engine)
    target_compile_options(engine PRIVATE
        -Wall
        -Wextra
        -Werror
        "$<$<COMPILE_LANGUAGE:OBJCXX>:-fobjc-arc>"
    )
    target_link_libraries(engine PUBLIC engine-core PRIVATE
        ${APPKIT_FRAMEWORK}
        ${METAL_FRAMEWORK}
        ${METALKIT_FRAMEWORK}
    )

    add_executable(engine-macos MACOSX_BUNDLE macOS/main.mm)
    set_target_properties(engine-macos PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.donnaysplash.engine"
        MACOSX_BUNDLE_BUNDLE_NAME "Engine"
    )
    target_compile_options(engine-macos PRIVATE
        -Wall
        -Wextra
        -Werror
        "$<$<COMPILE_LANGUAGE:OBJCXX>:-fobjc-arc>"
    )
    target_link_libraries(engine-macos PRIVATE engine ${APPKIT_FRAMEWORK})

    set(METAL_SHADER "${CMAKE_CURRENT_SOURCE_DIR}/macOS/Triangle.metal")
    set(METAL_AIR "${CMAKE_CURRENT_BINARY_DIR}/Triangle.air")
    set(METAL_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/default.metallib")

    add_custom_command(
        OUTPUT "${METAL_AIR}"
        COMMAND "${XCRUN_EXECUTABLE}" -sdk macosx metal -c "${METAL_SHADER}" -o "${METAL_AIR}"
        DEPENDS "${METAL_SHADER}"
        VERBATIM
    )
    add_custom_command(
        OUTPUT "${METAL_LIBRARY}"
        COMMAND "${XCRUN_EXECUTABLE}" -sdk macosx metallib "${METAL_AIR}" -o "${METAL_LIBRARY}"
        DEPENDS "${METAL_AIR}"
        VERBATIM
    )
    set_source_files_properties("${METAL_LIBRARY}" PROPERTIES
        GENERATED TRUE
        HEADER_FILE_ONLY TRUE
        MACOSX_PACKAGE_LOCATION Resources
    )
    target_sources(engine-macos PRIVATE "${METAL_LIBRARY}")
else()
    message(STATUS "Building the portable engine core only; the Metal host requires macOS.")
endif()
