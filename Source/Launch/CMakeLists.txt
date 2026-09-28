set(MODULE_NAME Launch)

set(MODULE_SRC
	Launch.cpp
)

add_executable(${MODULE_NAME}
	${MODULE_SRC}
)

target_link_libraries(${MODULE_NAME} PRIVATE
    CZRuntimeLibs CZEditor
)

target_link_options(${MODULE_NAME} PRIVATE -rdynamic "-Wl,-rpath,@executable_path")

# Runtime dependencies that are copied next to the libraries inside the .app bundle. Files that
# do not exist for the current SDK layout are reported once and skipped instead of breaking the
# build, so Homebrew / LunarG / Windows layouts all work.
set(CHOZO_BUNDLE_EXTRA_FILES "")

macro(chozo_bundle_file file description)
    if(EXISTS "${file}")
        list(APPEND CHOZO_BUNDLE_EXTRA_FILES "${file}")
        message(STATUS "CHOZO bundle: staging ${description} (${file})")
    else()
        message(STATUS "CHOZO bundle: ${description} not found at '${file}', skipping")
    endif()
endmacro()

if(APPLE AND CHOZO_VULKAN_LIB_DIR)
    chozo_bundle_file("${CHOZO_VULKAN_LIB_DIR}/libvulkan.dylib" "Vulkan loader")
    chozo_bundle_file("${CHOZO_VULKAN_LIB_DIR}/libvulkan.1.dylib" "Vulkan loader (versioned)")
    chozo_bundle_file("${CHOZO_VULKAN_LIB_DIR}/libMoltenVK.dylib" "MoltenVK")
    chozo_bundle_file("${CHOZO_MOLTENVK_ICD_FILE}" "MoltenVK ICD manifest")
endif()

set(CHOZO_BUNDLE_COPY_COMMANDS "")
foreach(extra_file IN LISTS CHOZO_BUNDLE_EXTRA_FILES)
    list(APPEND CHOZO_BUNDLE_COPY_COMMANDS
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${extra_file}"
                "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
    )
endforeach()

add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${BUNDLE_DIR}/Contents/Frameworks"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${BUNDLE_DIR}/Contents/Resources"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${BUNDLE_DIR}/Contents/MacOS"

    ${CHOZO_BUNDLE_COPY_COMMANDS}

    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/Resources/Shaders"
        "${BUNDLE_DIR}/Contents/Resources/Shaders"
    COMMENT "Copying Shaders to executable directory"

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/Info.plist"
        "${BUNDLE_DIR}/Contents/Info.plist"
    COMMENT "Assembling Chozo.app bundle"
)

set_target_properties(${MODULE_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${BUNDLE_DIR}/Contents/MacOS"
    BUILD_RPATH "@executable_path/../Frameworks"
    INSTALL_RPATH "@executable_path/../Frameworks"
)
