set(MODULE_NAME CZRHI)

set(MODULE_TYPE RUNTIME)

set(MODULE_INCLUDE
    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/RHITypes.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/RHIUtils.hpp

    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/GraphicsContext.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/RHIAPI.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/RHIResource.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/Device.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/Swapchain.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/Fence.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/Semaphore.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/CommandPool.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/CommandList.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/DescriptorSet.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/Texture.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/Image.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/Sampler.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/FrameBuffer.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/Pipeline.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/SetLayout.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/ShaderRes.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/GraphicsBuffer.hpp
)

set(MODULE_SRC
    RHIUtils.cpp

    GraphicsContext.cpp
    RHIAPI.cpp
    Device.cpp
    CommandList.cpp
    Texture.cpp
)

set(CZ_RUNTIME_RHI_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR})

# Precompiled header: the module ships RHIPCH.h next to its sources, and the module function
# picks up MODULE_PCH from here. Explicit rather than inferred, so which modules have one is
# visible in their own file.
set(MODULE_PCH "${CMAKE_CURRENT_SOURCE_DIR}/RHIPCH.h")

add_chozo_module(
    NAME    ${MODULE_NAME}
    TYPE    ${MODULE_TYPE}
    INCLUDE ${MODULE_INCLUDE}
    SOURCE  ${MODULE_SRC}
    LINK    CZCoreLibs
    PCH     RHIPCH.h
)

add_chozo_module_tests()