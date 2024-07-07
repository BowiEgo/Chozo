cmake_minimum_required(VERSION 3.28)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/tools/vcpkg/scripts/buildsystems/vcpkg.cmake")

project(Chozo VERSION 1.0.0)

message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add macro GL_SILENCE_DEPRECATION to ignore warnings about deprecated OpenGL API on MacOS.
add_definitions(-DGL_SILENCE_DEPRECATION)
add_definitions(-DCZ_ENABLE_ASSERTS)

if(WIN32)
    message("Platform: Windows")
    add_definitions(-DENGINE_PLATFORM_WIN32)
elseif(UNIX)
    if(APPLE)
        message("Platform: Macos")
        add_definitions(-DENGINE_PLATFORM_MACOS)
    else()
        message("Platform: Linux")
        add_definitions(-DENGINE_PLATFORM_LINUX)
    endif()
else()
    message("Platform: Unsupported")
endif()

#
# ChozoEngine static library
#
file(GLOB_RECURSE chozo_sources RELATIVE ${PROJECT_SOURCE_DIR} ChozoEngine/src/*.cpp)
file(GLOB_RECURSE chozo_vendor RELATIVE ${PROJECT_SOURCE_DIR} ChozoEngine/vendor/*.cpp)
add_library(${PROJECT_NAME} STATIC
  ${chozo_sources}
  ${chozo_vendor}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE CZ_PLATFORM_MAC)

# Set defines for build type
set(CMAKE_CXX_FLAGS_DEBUG -DCZ_DEBUG)
set(CMAKE_CXX_FLAGS_RELEASE -DCZ_RELEASE)
set(CMAKE_CXX_FLAGS_DIST -DCZ_DIST)

# Include directories for chozo
target_include_directories(${PROJECT_NAME} PUBLIC
  "${PROJECT_SOURCE_DIR}/ChozoEngine/src"
  "${PROJECT_SOURCE_DIR}/ChozoEngine/vendor/stb_image"
)

message(${PROJECT_SOURCE_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include/spdlog)

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
target_precompile_headers(${PROJECT_NAME} PRIVATE ChozoEngine/src/prxpch.h ChozoEngine/src/prxpch.cpp)

#
# Sandbox
#

# Define sandbox application
file(GLOB sandbox_sources sandbox/src/*.cpp)
add_executable(Sandbox ${sandbox_sources})

# Include directories for the sandbox application
target_include_directories(Sandbox PUBLIC "${PROJECT_SOURCE_DIR}/ChozoEngine/src")

# Link sandbox with the chozo library
target_link_libraries(Sandbox PUBLIC ${PROJECT_NAME})