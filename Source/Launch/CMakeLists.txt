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

add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${BUNDLE_DIR}/Contents/Frameworks"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${BUNDLE_DIR}/Contents/Resources"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${BUNDLE_DIR}/Contents/MacOS"

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CHOZO_VULKAN_LIB_DIR}/libvulkan.dylib"
        "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CHOZO_VULKAN_LIB_DIR}/libvulkan.1.dylib"
        "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CHOZO_VULKAN_LIB_DIR}/libMoltenVK.dylib"
        "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CHOZO_VULKAN_ICD_DIR}/MoltenVK_icd.json"
        "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
    
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
