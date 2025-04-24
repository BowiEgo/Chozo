project(ChozoEditor VERSION 1.0.0)

include(${CMAKE_SOURCE_DIR}/cmake/ResourceUtils.cmake)

# Source files
file(GLOB_RECURSE chozo_editor_sources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.cpp)
file(GLOB_RECURSE chozo_editor_headers RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.h)

if(APPLE)
    file(GLOB_RECURSE platform_utils ${MAC_PLATFORM_UTILS_PATH})
endif()

# add_executable(${PROJECT_NAME} ${chozo_editor_sources} ${chozo_editor_headers})
add_executable(${PROJECT_NAME} ${chozo_editor_sources} ${chozo_editor_headers} ${platform_utils})
set(EXECUTABLE_OUTPUT_PATH ${EXEC_PATH})

target_include_directories(${PROJECT_NAME} PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/src"
  "${CMAKE_SOURCE_DIR}/ChozoEngine/src"
)

target_link_libraries(${PROJECT_NAME} PUBLIC ChozoEngine)

define_target_resource_path(${PROJECT_NAME} TARGET_RESOURCES_DIR "resources")
copy_directory_to_parent(${PROJECT_NAME}
  ${CMAKE_CURRENT_SOURCE_DIR}/resources
  resources
)
create_directories(
    TARGET ${PROJECT_NAME}
    DIRS
        assets/HDRIs
        assets/Materials
        assets/Meshes
        assets/Scenes
        assets/Textures
        caches/shader
        caches/thumbnail
)
copy_platform_imgui_ini(${PROJECT_NAME})

# target_compile_definitions(${PROJECT_NAME} PRIVATE $<TARGET_PROPERTY:${ENGINE_PROJECT_NAME},COMPILE_DEFINITIONS>)