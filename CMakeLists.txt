# Helix RND — CMake Build Configuration
# Phase 1: Core architecture, helix.h, math, platform stubs
# Minimum CMake 3.20 for modern features

cmake_minimum_required(VERSION 3.20)
project(helix-rnd
    VERSION 1.0.0
    LANGUAGES CXX C
    DESCRIPTION "Helix RND — 2D/3D render engine with multi-language API"
    HOMEPAGE_URL "https://github.com/helix-rnd/helix"
)

# -----------------------------------------------------------------------------
# Policies & Standards
# -----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# -----------------------------------------------------------------------------
# Options
# -----------------------------------------------------------------------------
option(HX_BUILD_SHARED        "Build shared library (DLL/.so)" OFF)
option(HX_BUILD_STATIC        "Build static library" ON)
option(HX_STATIC_RUNTIME      "Link C/C++ runtime statically (/MT, -static-libstdc++)" ON)
option(HX_BUILD_TESTS         "Build tests" ON)
option(HX_BUILD_DEMO          "Build demo applications" ON)
option(HX_BUILD_HEADLESS_DEMO "Build headless triangle demo" ON)
option(HX_ENABLE_ASAN         "Enable AddressSanitizer" OFF)
option(HX_ENABLE_UBSAN        "Enable UndefinedBehaviorSanitizer" OFF)
option(HX_ENABLE_MSAN         "Enable MemorySanitizer (Linux only)" OFF)
option(HX_INSTALL_HEADERS     "Install public headers" ON)
option(HX_INSTALL_LIB         "Install library" ON)

# Backend selection (Phase 1: software only)
option(HX_BACKEND_VULKAN      "Enable Vulkan device discovery and backend" ON)
option(HX_BACKEND_OPENGL      "Enable OpenGL backend (Phase 4+)" OFF)
option(HX_BACKEND_METAL       "Enable Metal backend (Phase 8+)" OFF)
option(HX_BACKEND_SOFTWARE    "Enable software rasterizer backend" ON)

# Platform detection
if(WIN32)
    set(HX_PLATFORM_WINDOWS 1)
    add_definitions(-DHX_PLATFORM_WINDOWS=1)
elseif(APPLE)
    set(HX_PLATFORM_MACOS 1)
    add_definitions(-DHX_PLATFORM_MACOS=1)
elseif(UNIX AND NOT ANDROID)
    set(HX_PLATFORM_LINUX 1)
    add_definitions(-DHX_PLATFORM_LINUX=1)
elseif(EMSCRIPTEN)
    set(HX_PLATFORM_WASM 1)
    add_definitions(-DHX_PLATFORM_WASM=1)
endif()

# -----------------------------------------------------------------------------
# Compile Options
# -----------------------------------------------------------------------------
function(hx_apply_compile_options target)
    target_compile_options(${target} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:
            /W4
            /permissive-
            /Zc:__cplusplus
            /Zc:inline
            /Zc:throwingNew
            /external:W3
        >
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:
            -Wall -Wextra -Wpedantic
            -Wshadow -Wconversion -Wsign-conversion
            -Wnull-dereference -Wdouble-promotion
            -Wformat=2 -Wmisleading-indentation
            -Wduplicated-cond -Wduplicated-branches
            -Wlogical-op -Wnull-dereference
            -Wuseless-cast -Wold-style-cast
            -fno-common
        >
    )

    # Runtime library selection
    # For shared libraries on Windows, we MUST use dynamic runtime (/MD)
    # because static runtime (/MT) causes linking errors with CRT symbols.
    # For static libraries, we can use static runtime (/MT) if HX_STATIC_RUNTIME is ON.
    if(WIN32)
        get_target_property(target_type ${target} TYPE)
        if(target_type STREQUAL "SHARED_LIBRARY")
            # Shared library: use dynamic runtime
            target_compile_options(${target} PRIVATE /MD)
        elseif(HX_STATIC_RUNTIME)
            # Static library with static runtime
            target_compile_options(${target} PRIVATE /MT)
            target_link_options(${target} PRIVATE /NODEFAULTLIB:libcmt.lib)
        else()
            # Static library with dynamic runtime
            target_compile_options(${target} PRIVATE /MD)
        endif()
    elseif(HX_STATIC_RUNTIME AND NOT WIN32)
        # Non-Windows static runtime
        target_compile_options(${target} PRIVATE -static-libstdc++ -static-libgcc)
        target_link_options(${target} PRIVATE -static-libstdc++ -static-libgcc -pthread)
    endif()

    # Sanitizers
    if(HX_ENABLE_ASAN)
        target_compile_options(${target} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
        target_link_options(${target} PRIVATE -fsanitize=address)
    endif()
    if(HX_ENABLE_UBSAN)
        target_compile_options(${target} PRIVATE -fsanitize=undefined)
        target_link_options(${target} PRIVATE -fsanitize=undefined)
    endif()
    if(HX_ENABLE_MSAN AND UNIX AND NOT APPLE)
        target_compile_options(${target} PRIVATE -fsanitize=memory -fPIE)
        target_link_options(${target} PRIVATE -fsanitize=memory -pie)
    endif()

    # Debug symbols
    target_compile_options(${target} PRIVATE
        $<$<CONFIG:Debug>:$<$<CXX_COMPILER_ID:MSVC>:/Zi>/Zi>
        $<$<CONFIG:RelWithDebInfo>:$<$<CXX_COMPILER_ID:MSVC>:/Zi>/Zi>
    )
endfunction()

# -----------------------------------------------------------------------------
# Platform-specific libraries
# -----------------------------------------------------------------------------
function(hx_link_platform_libs target)
    if(WIN32)
        target_link_libraries(${target} PRIVATE
            user32 gdi32 shell32 ole32 uuid comctl32 version
        )
    elseif(APPLE)
        target_link_libraries(${target} PRIVATE
            "-framework Cocoa"
            "-framework Metal"
            "-framework QuartzCore"
            "-framework IOKit"
            "-framework CoreVideo"
        )
    elseif(UNIX AND NOT ANDROID)
        target_link_libraries(${target} PRIVATE
            dl pthread m rt X11
        )
    elseif(EMSCRIPTEN)
        # Web: no extra libs needed
    endif()
endfunction()

# -----------------------------------------------------------------------------
# Source Lists (used by both the shared build and the static components)
# -----------------------------------------------------------------------------
set(HELIX_CORE_SOURCES
    src/core/helix_core.cpp
    src/core/handle.cpp
    src/core/resource.cpp
    src/core/camera.cpp
    src/core/picture.cpp
    src/core/model.cpp
    src/core/sprite.cpp
    src/math/math.cpp
)
if(HX_BACKEND_VULKAN)
    list(APPEND HELIX_CORE_SOURCES
        src/core/gpu_vulkan.cpp
        src/thirdparty/volk/volk.c
    )
else()
    list(APPEND HELIX_CORE_SOURCES src/core/gpu_vulkan_stub.cpp)
endif()

set(HELIX_PLATFORM_SOURCES
    src/platform/platform.cpp
    src/platform/window.cpp
    src/platform/input.cpp
)
if(WIN32)
    list(APPEND HELIX_PLATFORM_SOURCES
        src/platform/win32/win32_window.cpp
        src/platform/win32/win32_input.cpp
    )
elseif(APPLE)
    list(APPEND HELIX_PLATFORM_SOURCES
        src/platform/cocoa/cocoa_window.mm
        src/platform/cocoa/cocoa_input.mm
    )
elseif(UNIX AND NOT ANDROID)
    list(APPEND HELIX_PLATFORM_SOURCES
        src/platform/x11/x11_window.cpp
        src/platform/x11/x11_input.cpp
        src/platform/wayland/wayland_window.cpp
        src/platform/wayland/wayland_input.cpp
    )
elseif(EMSCRIPTEN)
    list(APPEND HELIX_PLATFORM_SOURCES
        src/platform/wasm/wasm_window.cpp
        src/platform/wasm/wasm_input.cpp
    )
endif()
list(APPEND HELIX_PLATFORM_SOURCES
    src/platform/headless/headless_window.cpp
    src/platform/headless/headless_input.cpp
)

set(HELIX_RENDER_SOFTWARE_SOURCES
    src/render/software/soft_renderer.cpp
    src/render/software/soft_rasterizer.cpp
    src/render/software/soft_shader.cpp
    src/render/software/soft_texture.cpp
)

# -----------------------------------------------------------------------------
# Helix Library
#   HX_BUILD_SHARED=ON  -> single shared library (helix.dll / libhelix.so)
#   HX_BUILD_SHARED=OFF -> static components + interface target (default)
# -----------------------------------------------------------------------------
if(HX_BUILD_SHARED)

    add_library(helix SHARED
        ${HELIX_CORE_SOURCES}
        ${HELIX_PLATFORM_SOURCES}
    )
    if(HX_BACKEND_SOFTWARE)
        target_sources(helix PRIVATE ${HELIX_RENDER_SOFTWARE_SOURCES})
    endif()

    target_include_directories(helix
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    target_include_directories(helix SYSTEM PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/thirdparty/stb
        ${CMAKE_CURRENT_SOURCE_DIR}/src/thirdparty
    )
    if(HX_BACKEND_VULKAN)
        target_include_directories(helix SYSTEM PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src/thirdparty/vulkan/include
            ${CMAKE_CURRENT_SOURCE_DIR}/src/thirdparty/volk
        )
        target_compile_definitions(helix PRIVATE HX_HAS_VULKAN=1)
        target_link_libraries(helix PRIVATE ${CMAKE_DL_LIBS})
    endif()
    target_compile_definitions(helix PRIVATE
        HX_BUILD_DLL=1
        HX_CORE_EXPORTS
    )
    hx_apply_compile_options(helix)
    hx_link_platform_libs(helix)
    set_target_properties(helix PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
    )
    if(WIN32)
        # Native Windows naming: helix.dll (not libhelix.dll), so that
        # DllImport("helix") and standard Windows importers resolve it.
        set_target_properties(helix PROPERTIES PREFIX "")
    endif()

else()

# -----------------------------------------------------------------------------
# Helix Core Library (static component)
# -----------------------------------------------------------------------------
add_library(helix-core STATIC ${HELIX_CORE_SOURCES})

if(HX_BACKEND_VULKAN)
    target_include_directories(helix-core SYSTEM PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/thirdparty/vulkan/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src/thirdparty/volk
    )
    target_compile_definitions(helix-core PRIVATE HX_HAS_VULKAN=1)
    target_link_libraries(helix-core PRIVATE ${CMAKE_DL_LIBS})
endif()

target_include_directories(helix-core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_include_directories(helix-core SYSTEM PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/thirdparty/stb
    ${CMAKE_CURRENT_SOURCE_DIR}/src/thirdparty
)

target_compile_definitions(helix-core PRIVATE
    HX_BUILD_DLL=0
    HX_CORE_EXPORTS
)

hx_apply_compile_options(helix-core)

# -----------------------------------------------------------------------------
# Platform Abstraction Layer (static component)
# -----------------------------------------------------------------------------
add_library(helix-platform STATIC ${HELIX_PLATFORM_SOURCES})

target_include_directories(helix-platform
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(helix-platform PRIVATE helix-core)
hx_apply_compile_options(helix-platform)
hx_link_platform_libs(helix-platform)

# -----------------------------------------------------------------------------
# Render Backends (Phase 1: Software only)
# -----------------------------------------------------------------------------
if(HX_BACKEND_SOFTWARE)
    add_library(helix-render-software STATIC ${HELIX_RENDER_SOFTWARE_SOURCES})

    target_include_directories(helix-render-software
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(helix-render-software PRIVATE helix-core helix-platform)
    hx_apply_compile_options(helix-render-software)
endif()

# -----------------------------------------------------------------------------
# Main Helix Library (combines all) - INTERFACE library for header-only distribution
# -----------------------------------------------------------------------------
add_library(helix INTERFACE)

# Link all components into main library
target_link_libraries(helix INTERFACE
    helix-core
    helix-platform
)

if(HX_BACKEND_SOFTWARE)
    target_link_libraries(helix INTERFACE helix-render-software)
endif()

# Public API header
target_include_directories(helix
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# INTERFACE library properties
set_target_properties(helix PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

endif() # HX_BUILD_SHARED

# -----------------------------------------------------------------------------
# Headless Demo (Phase 1 deliverable)
# -----------------------------------------------------------------------------
if(HX_BUILD_HEADLESS_DEMO)
    add_executable(helix_demo_headless
        demos/headless_triangle/main.cpp
    )
    target_link_libraries(helix_demo_headless PRIVATE helix)
    hx_apply_compile_options(helix_demo_headless)
endif()

# -----------------------------------------------------------------------------
# Tests
# -----------------------------------------------------------------------------
if(HX_BUILD_TESTS)
    enable_testing()

    foreach(test_name IN ITEMS test_math test_handles test_error test_platform_stub test_pic test_resources test_gpu test_gltf test_sprite)
        add_executable(${test_name} tests/${test_name}.cpp)
        target_link_libraries(${test_name} PRIVATE helix)
        hx_apply_compile_options(${test_name})
        add_test(NAME ${test_name} COMMAND ${test_name})
    endforeach()

    add_executable(test_c_api tests/test_c_api.c)
    target_link_libraries(test_c_api PRIVATE helix)
    set_target_properties(test_c_api PROPERTIES LINKER_LANGUAGE CXX)
    add_test(NAME test_c_api COMMAND test_c_api)

    # Headless render test (image comparison)
    if(HX_BUILD_HEADLESS_DEMO)
        add_test(NAME helix_headless_triangle
            COMMAND helix_demo_headless
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
    endif()
endif()

# -----------------------------------------------------------------------------
# Installation
# -----------------------------------------------------------------------------
if(HX_INSTALL_HEADERS)
    install(FILES include/helix.h
        DESTINATION include
        COMPONENT headers
    )
endif()

if(HX_INSTALL_LIB)
    if(HX_BUILD_SHARED)
        install(TARGETS helix
            EXPORT helix-targets
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
            RUNTIME DESTINATION bin
            COMPONENT library
        )
    else()
        install(TARGETS helix helix-core helix-platform
            EXPORT helix-targets
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
            RUNTIME DESTINATION bin
            COMPONENT library
        )

        if(HX_BACKEND_SOFTWARE)
            install(TARGETS helix-render-software
                EXPORT helix-targets
                LIBRARY DESTINATION lib
                ARCHIVE DESTINATION lib
                RUNTIME DESTINATION bin
                COMPONENT library
            )
        endif()
    endif()

    install(EXPORT helix-targets
        FILE helix-config.cmake
        NAMESPACE helix::
        DESTINATION lib/cmake/helix
    )
endif()

# -----------------------------------------------------------------------------
# Packaging (CPack)
# -----------------------------------------------------------------------------
set(CPACK_PACKAGE_NAME "helix-rnd")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Helix RND — 2D/3D render engine")
set(CPACK_PACKAGE_VENDOR "Helix RND Team")
set(CPACK_PACKAGE_CONTACT "https://github.com/helix-rnd/helix")
set(CPACK_GENERATOR "ZIP;TGZ")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")

include(CPack)

# -----------------------------------------------------------------------------
# Export compile commands for clangd
# -----------------------------------------------------------------------------
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)