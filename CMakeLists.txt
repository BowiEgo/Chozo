cmake_minimum_required(VERSION 3.28)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/tools/vcpkg/scripts/buildsystems/vcpkg.cmake")

project(Chozo VERSION 1.0.0)

message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(ENGINE_PROJECT_NAME ChozoEngine)
set(EDITOR_PROJECT_NAME ChozoEditor)
set(SANDBOX_PROJECT_NAME Sandbox)

set(LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/${ENGINE_PROJECT_NAME}/lib)
set(EXEC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/${EDITOR_PROJECT_NAME}/bin)
set(HEAD_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIB_PATH})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${LIB_PATH}/Debug)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${LIB_PATH}/Release)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIB_PATH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${LIB_PATH}/Debug)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${LIB_PATH}/Release)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${EXEC_PATH})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${EXEC_PATH}/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${EXEC_PATH}/Release)

set(MAC_PLATFORM_UTILS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${ENGINE_PROJECT_NAME}/Platform/Mac/*.mm)

add_subdirectory(${ENGINE_PROJECT_NAME})
add_subdirectory(${EDITOR_PROJECT_NAME})
add_subdirectory(${SANDBOX_PROJECT_NAME})

set(RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${EDITOR_PROJECT_NAME}/resources)
set(SHADERS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${ENGINE_PROJECT_NAME}/shaders)
target_compile_definitions(${ENGINE_PROJECT_NAME} PRIVATE RESOURCES_DIR="${RESOURCES_DIR}")
target_compile_definitions(${ENGINE_PROJECT_NAME} PRIVATE SHADERS_DIR="${SHADERS_DIR}")

if(WIN32)
    message("//----------------- Platform: Windows -----------------//")
    set(platform_definitions CZ_PLATFORM_WIN)
elseif(UNIX)
    if(APPLE)
        message("//----------------- Platform: MacOS -----------------//")
        set(platform_definitions CZ_PLATFORM_MACOS)
    else()
        message("//----------------- Platform: Linux -----------------//")
        set(platform_definitions CZ_PLATFORM_LINUX)
    endif()
else()
    message("//----------------- Platform: Unsupported -----------------//")
    set(platform_definitions CZ_PLATFORM_UNSUPPORTED)
endif()

target_compile_definitions(${ENGINE_PROJECT_NAME} PRIVATE ${platform_definitions})
target_compile_definitions(${EDITOR_PROJECT_NAME} PRIVATE ${platform_definitions})
target_compile_definitions(${SANDBOX_PROJECT_NAME} PRIVATE ${platform_definitions})
