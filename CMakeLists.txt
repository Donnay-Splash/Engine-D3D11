cmake_minimum_required(VERSION 3.20)

set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "macOS architectures")
set(CMAKE_OSX_DEPLOYMENT_TARGET "13.1" CACHE STRING "Minimum macOS deployment target")

project(EngineMacOS LANGUAGES CXX)

if(NOT APPLE)
    message(FATAL_ERROR "The EngineMacOS CMake project only supports macOS.")
endif()

enable_language(OBJCXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_OBJCXX_STANDARD 11)
set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)
set(CMAKE_OBJCXX_EXTENSIONS OFF)

find_library(APPKIT_FRAMEWORK AppKit REQUIRED)
find_library(METAL_FRAMEWORK Metal REQUIRED)
find_library(METALKIT_FRAMEWORK MetalKit REQUIRED)

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
target_link_libraries(engine-macos PRIVATE
    ${APPKIT_FRAMEWORK}
    ${METAL_FRAMEWORK}
    ${METALKIT_FRAMEWORK}
)
