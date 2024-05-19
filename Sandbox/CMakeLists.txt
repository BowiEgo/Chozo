cmake_minimum_required(VERSION 3.28)
project(Sandbox VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

get_filename_component(PARENT_DIR ${CMAKE_SOURCE_DIR} DIRECTORY)

find_library(PARALLAX_ENGINE_LIBRARY
    NAMES Parallax
    PATHS ${PARENT_DIR}/ParallaxEngine/build/lib
    NO_DEFAULT_PATH
)

set(PARALLAX_ENGINE_DIR ${PARENT_DIR}/ParallaxEngine)

message(${PARALLAX_ENGINE_DIR})

if(NOT PARALLAX_ENGINE_LIBRARY)
    message(FATAL_ERROR ${PARALLAX_ENGINE_LIBRARY})
endif()

add_executable(Sandbox src/SandboxApp.cpp)

target_link_libraries(Sandbox PRIVATE ${PARALLAX_ENGINE_LIBRARY})
target_include_directories(Sandbox PRIVATE ${PARALLAX_ENGINE_DIR}/src)

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(Sandbox PUBLIC spdlog::spdlog)