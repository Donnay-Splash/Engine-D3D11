cmake_minimum_required(VERSION 3.20)

set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "macOS architectures")
set(CMAKE_OSX_DEPLOYMENT_TARGET "26" CACHE STRING "Minimum macOS deployment target")

project(EngineMacOS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/engine/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/engine/*.h"
)
list(FILTER ENGINE_SOURCES EXCLUDE REGEX "/engine/tests/")
add_library(engine STATIC ${ENGINE_SOURCES})
target_include_directories(engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/engine")
target_compile_options(engine PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall>
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wextra>
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Werror>
)

get_target_property(ENGINE_SOURCES engine SOURCES)
get_target_property(ENGINE_LINK_LIBRARIES engine LINK_LIBRARIES)
get_target_property(ENGINE_INTERFACE_LINK_LIBRARIES engine INTERFACE_LINK_LIBRARIES)
foreach(ENGINE_SOURCE IN LISTS ENGINE_SOURCES)
    if(ENGINE_SOURCE MATCHES "\\.(mm|m)$")
        message(FATAL_ERROR "The engine library must contain only portable C++ sources: ${ENGINE_SOURCE}")
    endif()
endforeach()
if(ENGINE_LINK_LIBRARIES OR ENGINE_INTERFACE_LINK_LIBRARIES)
    message(FATAL_ERROR "The engine library must not link platform or renderer libraries.")
endif()

if(BUILD_TESTING)
    add_subdirectory(engine/tests)
endif()

add_subdirectory(tools/ShaderCompiler)

if(APPLE)
    if(CMAKE_VERSION VERSION_LESS 3.25)
        message(FATAL_ERROR "The Metal development scheme requires CMake 3.25 or newer to enable Metal API Validation.")
    endif()

    enable_language(OBJCXX)
    set(CMAKE_OBJCXX_STANDARD 11)
    set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)
    set(CMAKE_OBJCXX_EXTENSIONS OFF)

    find_library(APPKIT_FRAMEWORK AppKit REQUIRED)
    find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)
    find_library(METAL_FRAMEWORK Metal REQUIRED)
    find_library(METALKIT_FRAMEWORK MetalKit REQUIRED)
    find_library(QUARTZCORE_FRAMEWORK QuartzCore REQUIRED)

    file(GLOB_RECURSE METAL_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/backends/metal/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/backends/metal/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/backends/metal/*.mm"
        "${CMAKE_CURRENT_SOURCE_DIR}/backends/metal/*.m"
    )
    list(FILTER METAL_SOURCES EXCLUDE REGEX "/backends/metal/tests/")
    add_library(engine-backend-metal STATIC ${METAL_SOURCES})

    target_include_directories(engine-backend-metal PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/backends/metal"
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party/metal-cpp"
    )
    target_compile_options(engine-backend-metal PRIVATE
        -Wall
        -Wextra
        -Werror
    )
    foreach(METAL_SOURCE IN LISTS METAL_SOURCES)
        if(METAL_SOURCE MATCHES "\\.(mm|m)$")
            message(FATAL_ERROR "The Metal backend must contain only C++ sources: ${METAL_SOURCE}")
        endif()
    endforeach()
    target_link_libraries(engine-backend-metal
        PUBLIC engine
        PRIVATE ${FOUNDATION_FRAMEWORK} ${METAL_FRAMEWORK} ${QUARTZCORE_FRAMEWORK}
    )

    add_executable(engine-macos MACOSX_BUNDLE macOS/main.mm)
    set_target_properties(engine-macos PROPERTIES
        MACOSX_BUNDLE_BUNDLE_NAME "Engine"
        XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.donnaysplash.engine"
        XCODE_GENERATE_SCHEME YES
        XCODE_SCHEME_ENABLE_GPU_API_VALIDATION YES
    )
    target_compile_options(engine-macos PRIVATE
        -fobjc-arc
        -Wall
        -Wextra
        -Werror
    )
    target_link_libraries(engine-macos
        PRIVATE engine-backend-metal ${APPKIT_FRAMEWORK} ${METALKIT_FRAMEWORK}
    )

    # Shaders are built by the shader-compiler tool (see tools/ShaderCompiler), not by CMake.
    # Run it before building; its metallibs are packaged into the app bundle and used by tests.
    set(SHADER_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build/shaders" CACHE PATH
        "Directory the shader-compiler tool writes its output to")
    file(MAKE_DIRECTORY "${SHADER_OUTPUT_DIR}/metal")
    add_custom_command(TARGET engine-macos POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_directory
            "${SHADER_OUTPUT_DIR}/metal"
            "$<TARGET_BUNDLE_CONTENT_DIR:engine-macos>/Resources/shaders"
        VERBATIM
    )

    if(BUILD_TESTING)
        add_executable(engine-metal-integration-tests backends/metal/tests/metal_backend_tests.cpp)
        target_link_libraries(engine-metal-integration-tests PRIVATE engine-backend-metal)
        add_dependencies(engine-metal-integration-tests engine-macos)
        target_compile_options(engine-metal-integration-tests PRIVATE -Wall -Wextra -Werror)
        target_compile_definitions(engine-metal-integration-tests PRIVATE
            METAL_SHADER_DIRECTORY="${SHADER_OUTPUT_DIR}/metal"
        )
        set_target_properties(engine-metal-integration-tests PROPERTIES
            XCODE_GENERATE_SCHEME YES
            XCODE_SCHEME_ENABLE_GPU_API_VALIDATION YES
        )
        add_test(NAME metal-backend-integration COMMAND engine-metal-integration-tests)
    endif()
else()
    message(STATUS "Building the portable engine library only; the Metal host requires macOS.")
endif()
