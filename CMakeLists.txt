cmake_minimum_required(VERSION 3.28)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/tools/vcpkg/scripts/buildsystems/vcpkg.cmake")

project(Parallax VERSION 1.0.0)

message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add macro GL_SILENCE_DEPRECATION to ignore warnings about deprecated OpenGL API on MacOS.
add_definitions(-DGL_SILENCE_DEPRECATION)
add_definitions(-DPRX_ENABLE_ASSERTS)

#
# ParallaxEngine static library
#
file(GLOB_RECURSE parallax_sources RELATIVE ${PROJECT_SOURCE_DIR} ParallaxEngine/src/*.cpp)
add_library(${PROJECT_NAME} STATIC
  ${parallax_sources}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE PRX_PLATFORM_MAC)

# Set defines for build type
set(CMAKE_CXX_FLAGS_DEBUG -DPRX_DEBUG)
set(CMAKE_CXX_FLAGS_RELEASE -DPRX_RELEASE)
set(CMAKE_CXX_FLAGS_DIST -DPRX_DIST)

message(${PROJECT_SOURCE_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include/spdlog)

# Include directories for parallax
target_include_directories(${PROJECT_NAME} PUBLIC
  "${PROJECT_SOURCE_DIR}/ParallaxEngine/src"
)

find_package(spdlog CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME} PUBLIC spdlog::spdlog_header_only)
target_link_libraries(${PROJECT_NAME} PRIVATE
                                      OpenGL::GL
                                      glfw
                                      glad::glad
                                      imgui::imgui
                                      glm::glm-header-only
)

# Precompiled header
target_precompile_headers(${PROJECT_NAME} PRIVATE ParallaxEngine/src/prxpch.h ParallaxEngine/src/prxpch.cpp)

#
# Sandbox
#

# Define sandbox application
file(GLOB sandbox_sources sandbox/src/*.cpp)
add_executable(Sandbox ${sandbox_sources})

# Include directories for the sandbox application
target_include_directories(Sandbox PUBLIC "${PROJECT_SOURCE_DIR}/ParallaxEngine/src")

# Link sandbox with the parallax library
target_link_libraries(Sandbox PUBLIC ${PROJECT_NAME})