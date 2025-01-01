project(ChozoEngine VERSION 1.0.0)

# Add macro GL_SILENCE_DEPRECATION to ignore warnings about deprecated OpenGL API on MacOS.
# add_definitions(-DGL_SILENCE_DEPRECATION)

#
# ChozoEngine static library
#
if(WIN32)
    message("//----------------- Platform: Windows -----------------//")
    file(GLOB_RECURSE platform_sources "Platform/Windows/*.cpp")
    set(platform_definitions ENGINE_PLATFORM_WIN32)
elseif(UNIX)
    if(APPLE)
        message("//----------------- Platform: MacOS -----------------//")
        file(GLOB_RECURSE platform_sources "Platform/Mac/*.cpp")
        set(platform_definitions ENGINE_PLATFORM_MACOS)
    else()
        message("//----------------- Platform: Linux -----------------//")
        file(GLOB_RECURSE platform_sources "Platform/Linux/*.cpp")
        set(platform_definitions ENGINE_PLATFORM_LINUX)
    endif()
else()
    message("//----------------- Platform: Unsupported -----------------//")
    set(platform_sources "")
    set(platform_definitions ENGINE_PLATFORM_UNSUPPORTED)
endif()

file(GLOB_RECURSE chozo_engine_sources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.cpp)
file(GLOB_RECURSE chozo_engine_vendor RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} vendor/*.cpp)

add_library(${PROJECT_NAME} STATIC ${chozo_engine_sources} ${chozo_engine_vendor} ${platform_sources})
set(LIBRARY_OUTPUT_PATH ${LIB_PATH})

target_compile_definitions(${PROJECT_NAME} PRIVATE CZ_DEBUG)
target_compile_definitions(${PROJECT_NAME} PRIVATE WINDOW_WIDTH=1920 WINDOW_HEIGHT=1080)
target_compile_definitions(${PROJECT_NAME} PRIVATE ${platform_definitions})

# message(${PROJECT_SOURCE_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include/spdlog)

find_package(spdlog CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(imguizmo CONFIG REQUIRED)
find_package(unofficial-shaderc CONFIG REQUIRED)
find_package(spirv_cross_core CONFIG REQUIRED)
find_package(spirv_cross_glsl CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_path(TINYGLTF_INCLUDE_DIRS "tiny_gltf.h")

find_library(COCOA_LIBRARY Cocoa)

target_link_libraries(${PROJECT_NAME} PUBLIC
    ${COCOA_LIBRARY}
)
target_link_libraries(${PROJECT_NAME} PUBLIC spdlog::spdlog_header_only)
target_link_libraries(${PROJECT_NAME} PRIVATE
    OpenGL::GL
    glfw
    glad::glad
    imgui::imgui
    glm::glm-header-only
    yaml-cpp::yaml-cpp
    imguizmo::imguizmo
    unofficial::shaderc::shaderc
    spirv-cross-core
    spirv-cross-glsl
    assimp::assimp
)

# Include directories for chozo
target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb_image"
        "${CMAKE_CURRENT_SOURCE_DIR}/vendor/entt/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/vendor/libshaderc_util/include"
)
target_include_directories(${PROJECT_NAME} PUBLIC ${TINYGLTF_INCLUDE_DIRS})

# Precompiled header
target_precompile_headers(${PROJECT_NAME} PRIVATE
    src/czpch.h
    src/czpch.cpp
)

