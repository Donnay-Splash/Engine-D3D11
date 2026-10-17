cmake_minimum_required(VERSION 3.20)

set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "macOS architectures")
set(CMAKE_OSX_DEPLOYMENT_TARGET "13.1" CACHE STRING "Minimum macOS deployment target")

project(EngineMacOS LANGUAGES CXX)

if(NOT APPLE)
    message(FATAL_ERROR "The EngineMacOS CMake project only supports macOS.")
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(engine-macos macOS/main.cpp)
target_compile_options(engine-macos PRIVATE -Wall -Wextra -Werror)
