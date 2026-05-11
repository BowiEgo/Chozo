set(MODULE_NAME CZRHI)

set(MODULE_TYPE RUNTIME)

set(MODULE_INCLUDE
    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/RHITypes.h
    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/RHIUtils.h
    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/GraphicContext.h
    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/Device.h
    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/Swapchain.h
    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/Texture.h
    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/Image.h
    ${CHOZO_INCLUDE_DIR}/Runtime/RHI/RHIAPI.h
)

set(MODULE_SRC
    GraphicContext.cpp
    RHIUtils.cpp
    Device.cpp
    Swapchain.cpp
    Texture.cpp
    Image.cpp
    RHIAPI.cpp
)

add_chozo_module(
    NAME    ${MODULE_NAME}
    TYPE    ${MODULE_TYPE}
    INCLUDE ${MODULE_INCLUDE}
    SOURCE  ${MODULE_SRC}
    LINK    CZCoreLibs
    PCH     RHIPCH.h
)

add_chozo_module_tests()