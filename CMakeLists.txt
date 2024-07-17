cmake_minimum_required(VERSION 3.28)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/tools/vcpkg/scripts/buildsystems/vcpkg.cmake")

project(Chozo VERSION 1.0.0)

message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(EXEC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(HEAD_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(ENGINE_PROJECT_NAME ChozoEngine)
set(EDITOR_PROJECT_NAME ChozoEditor)
set(SANDBOX_PROJECT_NAME Sandbox)

add_subdirectory(${ENGINE_PROJECT_NAME})
add_subdirectory(${EDITOR_PROJECT_NAME})
add_subdirectory(${SANDBOX_PROJECT_NAME})
