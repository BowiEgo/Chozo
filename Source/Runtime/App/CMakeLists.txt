set(MODULE_NAME CZApp)

set(MODULE_INCLUDE
    ${CHOZO_INCLUDE_DIR}/Runtime/App/Application.h
    ${CHOZO_INCLUDE_DIR}/Runtime/App/Engine.h
)

set(MODULE_SRC
    Application.cpp
    Engine.cpp
)

add_chozo_module(
    NAME    ${MODULE_NAME}
    TYPE    RUNTIME
    INCLUDE ${MODULE_INCLUDE}
    SOURCE  ${MODULE_SRC}
    LINK    CZCoreLibs
)

add_chozo_module_tests()