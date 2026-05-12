cmake_minimum_required(VERSION 3.28)

project(Chozo LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

message(STATUS "CMAKE_CXX_COMPILER_ID:            ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "CMAKE_CXX_COMPILER:               ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_C_COMPILER_ID:              ${CMAKE_C_COMPILER_ID}")
message(STATUS "CMAKE_C_COMPILER:                 ${CMAKE_C_COMPILER}")

# Macros
add_compile_definitions(
    $<$<CONFIG:Debug>:CZ_DEBUG>
)

if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "CMAKE_BUILD_TYPE not set, defaulting to Release")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

message(STATUS "CMAKE_BUILD_TYPE:                 ${CMAKE_BUILD_TYPE}")

set(CPP_RTTI_ENABLED OFF CACHE INTERNAL "")

# Build Options
option(CZ_OPTION_COMPILE_WITH_PROFILER "" ON)
message(STATUS "CZ_OPTION_COMPILE_WITH_PROFILER:  ${CZ_OPTION_COMPILE_WITH_PROFILER}")

option(CZ_OPTION_BUILD_BENCHMARKS "" ON)
message(STATUS "CZ_OPTION_BUILD_BENCHMARKS:       ${CZ_OPTION_BUILD_BENCHMARKS}")

option(CZ_OPTION_EMBED_RUNTIME "" ON)
message(STATUS "CZ_OPTION_EMBED_RUNTIME:          ${CZ_OPTION_EMBED_RUNTIME}")

if (CZ_OPTION_COMPILE_WITH_PROFILER)
    set(TRACY_ENABLE ON CACHE BOOL "" FORCE)
    add_compile_definitions(TRACY_ENABLE=1)
else()
    set(TRACY_ENABLE OFF CACHE BOOL "" FORCE)
    add_compile_definitions(TRACY_ENABLE=0)
endif()

message(STATUS "CHOZO TRACY_ENABLE:              ${TRACY_ENABLE}")

# Build Environment
set(CHOZO_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Source)
set(CHOZO_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Include)
set(CHOZO_SCRIPTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Scripts)
set(CHOZO_EXTERNAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/External)
set(CHOZO_BUILD_DIR ${CMAKE_BINARY_DIR})
set(CHOZO_BUILD_EMBED_DIR ${CMAKE_BINARY_DIR}/Embed)
set(CHOZO_DYNAMIC_LIB_DIR ${CMAKE_BINARY_DIR}/dyLibs)

message(STATUS "CHOZO SOURCE DIR                 ${CHOZO_SOURCE_DIR}")
message(STATUS "CHOZO INCLUDE DIR                ${CHOZO_INCLUDE_DIR}")
message(STATUS "CHOZO EXTERNAL DIR               ${CHOZO_EXTERNAL_DIR}")
message(STATUS "CHOZO BUILD DIR                  ${CHOZO_BUILD_DIR}")
message(STATUS "CHOZO BUILD EMBED DIR            ${CHOZO_BUILD_EMBED_DIR}")

# Fetch dependencies via CMake FetchContent
include(CMake/FetchDependencies.cmake)

# Find dependencies via CMake find_package
include(CMake/FindDependencies.cmake)

# Utilities to manage modules
include(CMake/ChozoModule.cmake)

# Add test
include(CMake/ChozoModuleTests.cmake)

# Build extra libs
add_subdirectory(${CHOZO_EXTERNAL_DIR}/stb)

# Prerequisites
# - third-party libs for all modules（like fmt）
add_library(CZEnginePrerequisites INTERFACE)

target_include_directories(CZEnginePrerequisites INTERFACE
    ${CHOZO_INCLUDE_DIR}
    # ${CHOZO_SOURCE_DIR}
)

target_link_libraries(CZEnginePrerequisites INTERFACE fmt::fmt)

# Chozo Core Modules
# - strict dependency ordering
# - user facing API in ${CHOZO_INCLUDE_DIR}
set(CZ_CORE_MODULE_FOLDER "Core")
set(CZ_CORE_MODULE_LIST "" CACHE STRING "" FORCE)
add_subdirectory(Source/Core/Platform)
add_subdirectory(Source/Core/Log)
add_subdirectory(Source/Core/Memory)
add_subdirectory(Source/Core/DynamicLibrary)
add_subdirectory(Source/Core/Layer)

add_library(CZCoreLibs INTERFACE)
target_link_libraries(CZCoreLibs INTERFACE
    fmt::fmt
    ${CZ_CORE_MODULE_LIST}
)
target_include_directories(CZCoreLibs INTERFACE
    ${CHOZO_INCLUDE_DIR}
)

# Chozo Runtime
# - standalone game runtime that is shipped
# - links with CZCoreLibs
set(CZ_RUNTIME_MODULE_FOLDER "Runtime")
set(CZ_RUNTIME_MODULE_LIST "" CACHE STRING "" FORCE)
add_subdirectory(Source/Runtime/RHI)
# add_subdirectory(Source/Runtime/UI)
add_subdirectory(Source/Runtime/RenderCore)
add_subdirectory(Source/Runtime/Window)
add_subdirectory(Source/Runtime/App)

add_library(CZRuntimeLibs INTERFACE)
target_link_libraries(CZRuntimeLibs INTERFACE
    CZCoreLibs
    ${CZ_RUNTIME_MODULE_LIST}
)
target_include_directories(CZRuntimeLibs INTERFACE
    ${CHOZO_INCLUDE_DIR}
)

# Chozo Backend
# - links with CZCoreLibs
set(CZ_BACKEND_MODULE_FOLDER "Backend")
set(CZ_BACKEND_MODULE_LIST "" CACHE STRING "" FORCE)

add_subdirectory(Source/Backend/Vulkan)

add_library(CZBackendLibs INTERFACE)
target_link_libraries(CZBackendLibs INTERFACE
    CZCoreLibs
    ${CZ_BACKEND_MODULE_LIST}
)
target_include_directories(CZBackendLibs INTERFACE
    ${CHOZO_INCLUDE_DIR}
)

# Chozo Editor
# - links with CZCoreLibs
set(CZ_EDITOR_MODULE_FOLDER "Editor")
add_subdirectory(Source/Editor)

# Chozo Launch
set(CZ_LAUNCH_MODULE_FOLDER "Launch")
add_subdirectory(Source/Launch)

# include(CMake/EmbedRuntime.cmake)

# Chozo Test
# - standalone test framework
# - links with CZCoreLibs
set(CZ_TEST_MODULE_FOLDER "Test")
add_subdirectory(Source/Test)
