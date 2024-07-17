project(ChozoEngine VERSION 1.0.0)

# Add macro GL_SILENCE_DEPRECATION to ignore warnings about deprecated OpenGL API on MacOS.
add_definitions(-DGL_SILENCE_DEPRECATION)
add_definitions(-DCZ_ENABLE_ASSERTS)

if(WIN32)
    message("//----------------- Platform: Windows -----------------//")
    add_definitions(-DENGINE_PLATFORM_WIN32)
elseif(UNIX)
    if(APPLE)
        message("//----------------- Platform: MacOS -----------------//")
        add_definitions(-DENGINE_PLATFORM_MACOS)
    else()
        message("//----------------- Platform: Linux -----------------//")
        add_definitions(-DENGINE_PLATFORM_LINUX)
    endif()
else()
    message("//----------------- Platform: Unsupported -----------------//")
endif()

#
# ChozoEngine static library
#
file(GLOB_RECURSE chozo_engine_sources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.cpp)
file(GLOB_RECURSE chozo_engine_vendor RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} vendor/*.cpp)

add_library(${PROJECT_NAME} STATIC ${chozo_engine_sources} ${chozo_engine_vendor})
set(LIBRARY_OUTPUT_PATH ${LIB_PATH})

target_compile_definitions(${PROJECT_NAME} PRIVATE CZ_PLATFORM_MAC)
target_compile_definitions(${PROJECT_NAME} PRIVATE WINDOW_WIDTH=1600 WINDOW_HEIGHT=900)

# Include directories for chozo
target_include_directories(${PROJECT_NAME} PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/src"
  "${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb_image"
  "${CMAKE_CURRENT_SOURCE_DIR}/vendor/entt/include"
)

# message(${PROJECT_SOURCE_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include/spdlog)

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
target_precompile_headers(${PROJECT_NAME} PRIVATE
    src/czpch.h
    src/czpch.cpp
)

