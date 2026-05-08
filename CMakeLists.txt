cmake_minimum_required(VERSION 3.28)

project(Chozo LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "CMAKE_CXX_COMPILER_ID:            ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "CMAKE_CXX_COMPILER:               ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_C_COMPILER_ID:              ${CMAKE_C_COMPILER_ID}")
message(STATUS "CMAKE_C_COMPILER:                 ${CMAKE_C_COMPILER}")

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

# build extra libs
add_subdirectory(${CHOZO_EXTERNAL_DIR}/stb)

# Chozo Core Modules
# - strict dependency ordering
# - user facing API in ${CHOZO_INCLUDE_DIR}

set(CZ_CORE_MODULE_FOLDER "Core")
set(CZ_CORE_MODULE_LIST "" CACHE STRING "" FORCE)
add_subdirectory(Source/Core/Log)

add_library(CZCoreLibs INTERFACE)
target_link_libraries(CZCoreLibs INTERFACE
    fmt::fmt
    ${CZ_CORE_MODULE_LIST}
)

# Chozo Runtime
# - standalone game runtime that is shipped
# - links with CZCoreLibs

set(CZ_RUNTIME_MODULE_FOLDER "Launch")
add_subdirectory(Source/Runtime)

# include(CMake/EmbedRuntime.cmake)