set(MODULE_NAME CZApp)

set(MODULE_INCLUDE
    ${CHOZO_INCLUDE_DIR}/Runtime/App/Application.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/App/StartupHost.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/App/Engine.hpp
)

set(MODULE_SRC
    Application.cpp
    StartupHost.cpp
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