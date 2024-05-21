cmake_minimum_required(VERSION 3.28)

project(Parallax VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-fdeclspec)
endif()

get_filename_component(PARENT_DIR ${CMAKE_SOURCE_DIR} DIRECTORY)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/Parallax)

file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/Parallax/**.cpp")

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)

target_compile_definitions(${PROJECT_NAME} PUBLIC -DPARALLAX_BUILD_DLL)

if(CMAKE_SYSTEM_NAME MATCHES "WIN32")
    target_compile_definitions(${PROJECT_NAME} PUBLIC -D_PARALLAX_PLATFORM_WIN)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    target_compile_definitions(${PROJECT_NAME} PUBLIC -D_PARALLAX_PLATFORMMAC)
endif()

target_precompile_headers(${PROJECT_NAME} PRIVATE src/prxpch.h src/prxpch.cpp)

include_directories(${PARENT_DIR}/vendor/spdlog/include)
# find_package(spdlog CONFIG REQUIRED)
# target_link_libraries(Parallax PUBLIC spdlog::spdlog spdlog::spdlog_header_only)
