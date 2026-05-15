set(MODULE_NAME CZRenderCore)

set(MODULE_INCLUDE
    ${CHOZO_INCLUDE_DIR}/Runtime/RenderCore/Renderer.hpp
)

set(MODULE_SRC
    Renderer.cpp
)

add_chozo_module(
    NAME    ${MODULE_NAME}
    TYPE    RUNTIME
    INCLUDE ${MODULE_INCLUDE}
    SOURCE  ${MODULE_SRC}
    LINK    CZCoreLibs
)

add_chozo_module_tests()