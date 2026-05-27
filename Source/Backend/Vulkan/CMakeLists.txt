set(MODULE_NAME CZVulkan)

set(MODULE_INCLUDE
    VulkanAPIObj.hpp
    VulkanRenderContext.hpp
    VulkanGraphicsContextObj.hpp
    VulkanDeviceObj.hpp
    VulkanSwapchainObj.hpp
    VulkanFenceObj.hpp
    VulkanSemaphoreObj.hpp
    VulkanCommandPoolObj.hpp
    VulkanCommandBufferObj.hpp
    VulkanTextureObj.hpp
    VulkanImageObj.hpp
    VulkanSamplerObj.hpp
    VulkanFrameBufferObj.hpp
    VulkanShaderResObj.hpp
    VulkanSetLayoutObj.hpp
    VulkanPipelineObj.hpp
    VulkanGraphicsBufferObj.hpp
)

set(MODULE_SRC
    VmaImpl.cpp
    VulkanExternAPIs.cpp
    VulkanUtils.cpp
    VulkanAPIObj.cpp
    VulkanGraphicsContextObj.cpp
    VulkanDeviceObj.cpp
    VulkanSwapchainObj.cpp
    VulkanFenceObj.cpp
    VulkanSemaphoreObj.cpp
    VulkanCommandPoolObj.cpp
    VulkanCommandBufferObj.cpp
    VulkanTextureObj.cpp
    VulkanImageObj.cpp
    VulkanSamplerObj.cpp
    VulkanFrameBufferObj.cpp
    VulkanShaderResObj.cpp
    VulkanSetLayoutObj.cpp
    VulkanPipelineObj.cpp
    VulkanGraphicsBufferObj.cpp
)

add_chozo_module(
    NAME    ${MODULE_NAME}
    TYPE    BACKEND
    LIBRARY_TYPE  SHARED
    INCLUDE ${MODULE_INCLUDE} ${CZ_RUNTIME_RHI_INCLUDE}
    SOURCE  ${MODULE_SRC}
    LINK    Vulkan::Vulkan VulkanMemoryAllocator CZCoreLibs CZRHI
    PCH     VulkanPCH.h
)

# set_target_properties(${MODULE_NAME} PROPERTIES
#     LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/dyLibs"
#     RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/dyLibs"
# )

add_chozo_module_tests()