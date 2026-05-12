include(FetchContent)

## Fetch doctest
FetchContent_Declare(
  doctest
  GIT_REPOSITORY https://github.com/doctest/doctest.git
  GIT_TAG v2.4.11 
)
FetchContent_MakeAvailable(doctest)


## Fetch fmt
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 10.2.1
)
FetchContent_MakeAvailable(fmt)

## Fetch spdlog
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.17.0
)
FetchContent_MakeAvailable(spdlog)
message(STATUS "CHOZO SPDLOG SOURCE DIR:     ${spdlog_SOURCE_DIR}")

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
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG docking
)
FetchContent_MakeAvailable(imgui)
message(STATUS "CHOZO IMGUI SOURCE DIR:      ${imgui_SOURCE_DIR}")

FetchContent_GetProperties(imgui SOURCE_DIR imgui_SOURCE_DIR)
if(NOT imgui_SOURCE_DIR)
    message(FATAL_ERROR "ImGui source directory not found")
endif()

set(IMGUI_CORE_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
)

set(IMGUI_BACKEND_SOURCES
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
)

add_library(imgui STATIC ${IMGUI_CORE_SOURCES} ${IMGUI_BACKEND_SOURCES})

target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${imgui_SOURCE_DIR}/misc/cpp
)

target_compile_definitions(imgui PUBLIC IMGUI_DEFINE_MATH_OPERATORS)

find_package(Vulkan REQUIRED)
target_link_libraries(imgui PUBLIC Vulkan::Vulkan SDL3::SDL3)

target_precompile_headers(imgui PRIVATE ${imgui_SOURCE_DIR}/imgui.h)

add_library(imgui::imgui ALIAS imgui)
