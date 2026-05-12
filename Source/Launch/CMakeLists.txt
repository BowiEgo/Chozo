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

target_link_options(${MODULE_NAME} PRIVATE -rdynamic)

add_custom_command(TARGET Launch POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CHOZO_VULKAN_LIB_DIR}/libvulkan.dylib"
        "$<TARGET_FILE_DIR:Launch>"

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CHOZO_VULKAN_LIB_DIR}/libMoltenVK.dylib"
        "$<TARGET_FILE_DIR:Launch>"

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CHOZO_VULKAN_ICD_DIR}/MoltenVK_icd.json"
        "$<TARGET_FILE_DIR:Launch>"
)
