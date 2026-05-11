set(MODULE_NAME CZVulkan)

set(MODULE_INCLUDE
    VulkanGraphicContextObj.h
    VulkanDeviceObj.h
    VulkanSwapchainObj.h
    VulkanTextureObj.h
    VulkanImageObj.h
)

set(MODULE_SRC
    VmaImpl.cpp
    VulkanGraphicContextObj.cpp
    VulkanDeviceObj.cpp
    VulkanSwapchainObj.cpp
    VulkanTextureObj.cpp
    VulkanImageObj.cpp
    VulkanUtils.cpp
)

add_chozo_module(
    NAME    ${MODULE_NAME}
    TYPE    BACKEND
    LIBRARY_TYPE  SHARED
    INCLUDE ${MODULE_INCLUDE}
    SOURCE  ${MODULE_SRC}
    LINK    Vulkan::Vulkan VulkanMemoryAllocator CZCoreLibs CZRHI
    PCH     VulkanPCH.h
)

# set_target_properties(${MODULE_NAME} PROPERTIES
#     LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/dyLibs"
#     RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/dyLibs"
# )

add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:${MODULE_NAME}>"
        "$<TARGET_FILE_DIR:Launch>"
)

add_chozo_module_tests()