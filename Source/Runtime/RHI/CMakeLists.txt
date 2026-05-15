set(MODULE_NAME CZRHI)

set(MODULE_TYPE RUNTIME)

set(MODULE_INCLUDE
    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/RHITypes.h
    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/RHIUtils.h

    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/GraphicsContext.h
    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/RHIAPI.h
    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/Device.h
    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/Swapchain.h
    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/Fence.h
    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/Semaphore.h
    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/CommandPool.h
    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/CommandList.h
    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/Texture.h
    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/Image.h
    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/Sampler.h
)

set(MODULE_SRC
    RHIUtils.cpp

    GraphicsContext.cpp
    RHIAPI.cpp
    Device.cpp
    Swapchain.cpp
    Fence.cpp
    Semaphore.cpp
    CommandPool.cpp
    CommandList.cpp
    Texture.cpp
    Image.cpp
    Sampler.cpp
)

set(CZ_RUNTIME_RHI_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR})

add_chozo_module(
    NAME    ${MODULE_NAME}
    TYPE    ${MODULE_TYPE}
    INCLUDE ${MODULE_INCLUDE}
    SOURCE  ${MODULE_SRC}
    LINK    CZCoreLibs
    PCH     RHIPCH.h
)

add_chozo_module_tests()