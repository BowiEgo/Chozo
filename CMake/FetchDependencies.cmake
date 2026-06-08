include(FetchContent)

# Fetch doctest
FetchContent_Declare(
  doctest
  GIT_REPOSITORY https://github.com/doctest/doctest.git
  GIT_TAG v2.4.11 
)
FetchContent_MakeAvailable(doctest)
message(STATUS "CHOZO DOCTEST SOURCE DIR:     ${doctest_SOURCE_DIR}")

## Fetch fmt
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 10.2.1
)
FetchContent_MakeAvailable(fmt)
message(STATUS "CHOZO FMT SOURCE DIR:     ${fmt_SOURCE_DIR}")

## Fetch spdlog
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.17.0
)
FetchContent_MakeAvailable(spdlog)
message(STATUS "CHOZO SPDLOG SOURCE DIR:     ${spdlog_SOURCE_DIR}")

## Fetch glm
FetchContent_Declare(
  glm
  URL https://github.com/g-truc/glm/releases/download/1.0.3/glm-1.0.3.zip
  URL_HASH SHA256=1c0a0fced9b0d87c7b7bc94e40be490cff6d4c83c25db8488d8f33754e7fdeb2
)
FetchContent_MakeAvailable(glm)
message(STATUS "CHOZO GLM SOURCE DIR:     ${glm_SOURCE_DIR}")

## Fetch entt
FetchContent_Declare(
  entt
  GIT_REPOSITORY https://github.com/skypjack/entt.git
  GIT_TAG v3.16.0
)
FetchContent_MakeAvailable(entt)
message(STATUS "CHOZO ENTT SOURCE DIR:     ${entt_SOURCE_DIR}")

## Fetch SDL3
FetchContent_Declare(
  sdl3
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG release-3.4.8
)

set(SDL3_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(SDL3_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SDL3_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_DISABLE_INSTALL_DOCS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(sdl3)
message(STATUS "CHOZO SDL3 SOURCE DIR:     ${sdl3_SOURCE_DIR}")

## Fetch Vulkan Memory Allocator
FetchContent_Declare(
  vma
  GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
  GIT_TAG v3.3.0
)
FetchContent_MakeAvailable(vma)

if(TARGET GPUOpen::VulkanMemoryAllocator)
  message(STATUS "Found target: GPUOpen::VulkanMemoryAllocator")
elseif(TARGET VulkanMemoryAllocator)
  message(STATUS "Found target: VulkanMemoryAllocator")
else()
  message(WARNING "Could not find a known target for VMA.")
endif()

message(STATUS "CHOZO VMA SOURCE DIR:      ${vma_SOURCE_DIR}")

## Fetch ImGui
FetchContent_Declare(
  ChozoImGui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG docking
  UPDATE_DISCONNECTED TRUE
)
FetchContent_MakeAvailable(ChozoImGui)

FetchContent_GetProperties(ChozoImGui SOURCE_DIR ChozoImGui_SOURCE_DIR)
if(NOT ChozoImGui_SOURCE_DIR)
    message(FATAL_ERROR "ImGui source directory not found")
endif()

set(IMGUI_CORE_SOURCES
    ${ChozoImGui_SOURCE_DIR}/imgui.cpp
    ${ChozoImGui_SOURCE_DIR}/imgui_draw.cpp
    ${ChozoImGui_SOURCE_DIR}/imgui_tables.cpp
    ${ChozoImGui_SOURCE_DIR}/imgui_widgets.cpp
    ${ChozoImGui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
)

set(IMGUI_BACKEND_SOURCES
    ${ChozoImGui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
    ${ChozoImGui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
)

add_library(ChozoImGui STATIC ${IMGUI_CORE_SOURCES} ${IMGUI_BACKEND_SOURCES})

target_include_directories(ChozoImGui PUBLIC
    ${ChozoImGui_SOURCE_DIR}
    ${ChozoImGui_SOURCE_DIR}/backends
    ${ChozoImGui_SOURCE_DIR}/misc/cpp
)

target_compile_definitions(ChozoImGui PUBLIC IMGUI_DEFINE_MATH_OPERATORS)

find_package(Vulkan REQUIRED)
target_link_libraries(ChozoImGui PUBLIC Vulkan::Vulkan SDL3::SDL3)

target_precompile_headers(ChozoImGui PRIVATE ${ChozoImGui_SOURCE_DIR}/imgui.h)

add_library(Chozo::ImGui ALIAS ChozoImGui)

message(STATUS "CHOZO IMGUI SOURCE DIR:      ${ChozoImGui_SOURCE_DIR}")

## Fetch Slang
set(SLANG_VERSION "2026.9.1") 

# check platform and construct download URL
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
        set(SLANG_PLATFORM "windows-x86_64")
        set(SLANG_ARCHIVE_SUFFIX ".zip")
    elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "ARM64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
        set(SLANG_PLATFORM "windows-aarch64")
        set(SLANG_ARCHIVE_SUFFIX ".zip")
    else()
        message(FATAL_ERROR "Unsupported Windows architecture: ${CMAKE_SYSTEM_PROCESSOR}")
    endif()

elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
        set(SLANG_PLATFORM "macos-x86_64")
        set(SLANG_ARCHIVE_SUFFIX ".zip")
    elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
        set(SLANG_PLATFORM "macos-aarch64")
        set(SLANG_ARCHIVE_SUFFIX ".zip")
    else()
        message(FATAL_ERROR "Unsupported macOS architecture: ${CMAKE_SYSTEM_PROCESSOR}")
    endif()

elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
        set(SLANG_PLATFORM "linux-x86_64")
        set(SLANG_ARCHIVE_SUFFIX ".tar.gz")
    elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
        set(SLANG_PLATFORM "linux-aarch64")
        set(SLANG_ARCHIVE_SUFFIX ".tar.gz")
    else()
        message(FATAL_ERROR "Unsupported Linux architecture: ${CMAKE_SYSTEM_PROCESSOR}")
    endif()

else()
    message(FATAL_ERROR "Unsupported system: ${CMAKE_SYSTEM_NAME}")
endif()

# 3. build download from URL
set(SLANG_ARCHIVE_NAME "slang-${SLANG_VERSION}-${SLANG_PLATFORM}${SLANG_ARCHIVE_SUFFIX}")
set(SLANG_DOWNLOAD_URL "https://github.com/shader-slang/slang/releases/download/v${SLANG_VERSION}/${SLANG_ARCHIVE_NAME}")

message(STATUS "Downloading Slang from: ${SLANG_DOWNLOAD_URL}")

FetchContent_Declare(
    slang_prebuilt
    URL ${SLANG_DOWNLOAD_URL}
)
FetchContent_MakeAvailable(slang_prebuilt)

FetchContent_GetProperties(slang_prebuilt)
if(NOT slang_prebuilt_POPULATED)
    FetchContent_Populate(slang_prebuilt)
endif()

set(SLANG_PREBUILT_DIR ${slang_prebuilt_SOURCE_DIR})

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(SLANG_LIBRARY ${SLANG_PREBUILT_DIR}/lib/slang.lib) # or slang.dll
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(SLANG_LIBRARY ${SLANG_PREBUILT_DIR}/lib/libslang.dylib)
  elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(SLANG_LIBRARY ${SLANG_PREBUILT_DIR}/lib/libslang.so)
endif()

add_library(slang::slang SHARED IMPORTED)
set_target_properties(slang::slang PROPERTIES
    IMPORTED_LOCATION ${SLANG_LIBRARY}
    INTERFACE_INCLUDE_DIRECTORIES ${SLANG_PREBUILT_DIR}/include
)

message(STATUS "CHOZO SLANG SOURCE DIR:      ${SLANG_PREBUILT_DIR}")

## Fetch Tracy Profiler
option(TRACY_ON_DEMAND "" ON)
option(TRACY_ENABLE "" ON)
option(TRACY_ONLY_LOCALHOST "" ON)
# option(TRACY_NO_EXIT "" OFF)
# option(TRACY_CALLSTACK "" OFF)

FetchContent_Declare(
  tracy
  GIT_REPOSITORY https://github.com/wolfpld/tracy.git
  GIT_TAG v0.13.1
)
FetchContent_MakeAvailable(tracy)
message(STATUS "CHOZO TRACY SOURCE DIR:    ${tracy_SOURCE_DIR}")
